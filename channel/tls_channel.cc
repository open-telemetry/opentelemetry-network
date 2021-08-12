//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <channel/callbacks.h>
#include <channel/component.h>
#include <channel/internal/ssl_context.h>
#include <channel/internal/tls_shim.h>
#include <channel/tls_channel.h>
#include <channel/tls_error.h>
#include <config.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <util/error_handling.h>
#include <util/log.h>

#include <iostream>
#include <sstream>

constexpr u32 APP_READ_SIZE = (16 * 1024);

int channel::TLSChannel::Initializer::channel_index = -1;

static bool verify_name(std::string_view peername, std::string_view certname)
{
  if (peername == certname) {
    LOG::trace_in(channel::Component::tls, "Successfully validated cert names. Got '{}', expected '{}'", certname, peername);
    return true;
  }
  // Support wildcard certificates
  if ("*." == certname.substr(0, 2)) {
    size_t subject_dot_pos = 1;

    std::string_view subject_domain = certname.substr(subject_dot_pos + 1);

    size_t peer_dot_pos = peername.find('.');
    if (peer_dot_pos == std::string::npos) {
      return false;
    }
    std::string_view peer_domain = peername.substr(peer_dot_pos + 1);

    if (subject_domain == peer_domain) {
      /* Everything after the wildcard compared equal */
      LOG::trace_in(channel::Component::tls, "Successfully validated cert names. Got '{}', expected '{}'", certname, peername);
      return true;
    } else {
      LOG::trace_in(
          channel::Component::tls,
          "Certificate verification with wildcards error: subject name differs:"
          " got '{}' expected '{}'",
          certname,
          peername);
      return false;
    }
  } else {
    /* not equal. reject */
    LOG::trace_in(
        channel::Component::tls,
        "Certificate verification error: subject name differs:"
        " got '{}' expected '{}'",
        certname,
        peername);
    return false;
  }
}

static int verify_callback(int preverify, X509_STORE_CTX *ctx)
{
  if (preverify != 1)
    return 0;

  int depth = X509_STORE_CTX_get_error_depth(ctx);
  /* we only want to validate the server's certificate, not the CA */
  if (depth != 0)
    return preverify;

  /* Get the ssl pointer from the context */
  SSL *ssl = (SSL *)X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());

  /* get the TLSChannel pointer from the SSL object */
  auto channel = (channel::TLSChannel *)SSL_get_ex_data(ssl, channel::TLSChannel::Initializer::channel_index);

  /* note: cert is an internal pointer (don't free) */
  X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
  if (cert == nullptr) {
    LOG::error("Certificate verification error: no peer certificate");
    return 0;
  }

  /* note: MUST NOT free subject_name */
  X509_NAME *subject_name = X509_get_subject_name(cert);
  if (subject_name == nullptr) {
    LOG::error("Certificate verification error: no subject name");
    return 0;
  }

  int idx = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
  if (idx < 0) {
    LOG::error("Certificate verification error: cannot get subject index");
    return 0;
  }

  /* seems like an internal pointer. the reference code doesn't free it
   *  (the .tar.gz at https://wiki.openssl.org/index.php/SSL/TLS_Client)
   */
  X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject_name, idx);
  if (entry == nullptr) {
    LOG::error("Certificate verification error: cannot get subject entry");
    return 0;
  }

  /* also looks like an internal pointer and not freed by the example */
  ASN1_STRING *data = X509_NAME_ENTRY_get_data(entry);
  if (data == nullptr) {
    LOG::error("Certificate verification error: cannot get subject data");
    return 0;
  }

  if (const std::string &peer_hostname = channel->peer_hostname(); peer_hostname.empty()) {
    return preverify;
  } else {
    u8 *subject = nullptr;
    int length = ASN1_STRING_to_UTF8(&subject, data);
    ASSUME(length >= 0);
    if (subject == nullptr) {
      LOG::error("Certificate verification error: cannot get subject string");
      return 0;
    }
    /* after this point, will need to OPENSSL_free(subject) */
    std::string_view subject_string(reinterpret_cast<char const *>(subject), length);

    if (verify_name(peer_hostname, subject_string)) {
      OPENSSL_free(subject);
      return preverify;
    } else {
      // Did not validate, free subject and move on to alternate names
      OPENSSL_free(subject);
    }

    GENERAL_NAMES *names = reinterpret_cast<GENERAL_NAMES *>(X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0));
    if (!names) {
      LOG::error("Certificate verification error: cannot get general names");
      return 0;
    }
    /* after this point, will need to GENERAL_NAMES_free(names) */
    int count = sk_GENERAL_NAME_num(names);
    if (!count) {
      LOG::error("Certificate verification error: cannot get general name count");
      GENERAL_NAMES_free(names);
      return 0;
    }
    for (int i = 0; i < count; ++i) {
      GENERAL_NAME *entry = sk_GENERAL_NAME_value(names, i);
      if (!entry) {
        LOG::error("Certificate verification error: could not get alternate name entry");
        GENERAL_NAMES_free(names);
        return 0;
      }
      if (GEN_DNS == entry->type) {
        u8 *name = nullptr;
        int name_len = 0;
        name_len = ASN1_STRING_to_UTF8(&name, entry->d.dNSName);
        ASSUME(name_len >= 0);
        if (!name) {
          LOG::error("Certificate verification error: could not get string value of alternate name entry");
          GENERAL_NAMES_free(names);
          return 0;
        }
        /* need to OPENSSL_free(name) after this point */
        std::string_view alt_name(reinterpret_cast<char const *>(name), name_len);
        if (verify_name(peer_hostname, alt_name)) {
          // Successfully validated name, return success
          OPENSSL_free(name);
          GENERAL_NAMES_free(names);
          return preverify;
        }
        if (name) {
          // Did not validate, continue on to next name
          OPENSSL_free(name);
          name = nullptr;
        }
      }
    }

    GENERAL_NAMES_free(names);
    LOG::error("Could not validate any of the hostnames in the cert.");
    return 0;
  }
}

/****************************
 * TLSChannel::Initializer
 ****************************/
channel::TLSChannel::Initializer::Initializer()
{
  static_assert(OPENSSL_VERSION_NUMBER == 0x1010102fL, "unexpected OpenSSL version");

  /* https://www.openssl.org/docs/ssl/SSL_library_init.html */
  int ret = OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
  if (ret != 1) {
    throw std::runtime_error("Failed to initialize TLS library");
  }

  channel_index = CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_SSL, 0, nullptr, nullptr, nullptr, nullptr);
  if (channel_index == -1) {
    throw std::runtime_error("could not allocate SSL index for channels");
  }
}

channel::TLSChannel::Initializer::~Initializer()
{
  /* free the index mapping SSL to TLSChannel */
  CRYPTO_free_ex_index(CRYPTO_EX_INDEX_SSL, channel_index);

  /* Deinit does not seem to be necessary in 1.1.0 onwards as discussed in:
   *   https://rt.openssl.org/Ticket/Display.html?id=3824&user=guest&pass=guest
   *
   * However if this is discovered to be incorrect, there should be info on:
   *   https://wiki.openssl.org/index.php/Library_Initialization
   *   (under Cleanup)
   */
  return;
}

/****************************
 * TLSChannel::Credentials
 ****************************/
channel::TLSChannel::Credentials::Credentials(std::string client_key, std::string client_crt)
    : client_key_(std::move(client_key)), client_crt_(std::move(client_crt))
{}

/****************************
 * TLSChannel
 ****************************/
channel::TLSChannel::TLSChannel(TCPChannel &transport, Credentials &creds, std::string server_hostname)
    : transport_(transport),
      creds_(creds),
      server_hostname_(std::move(server_hostname)),
      ssl_context_(std::make_unique<internal::SSLContext>(creds_.client_key_, creds.client_crt_))
{
  LOG::trace_in(channel::Component::tls, "TLSChannel - server hostname: '{}'", server_hostname_);
}

channel::TLSChannel::~TLSChannel()
{
  if (tls_shim_ && !tls_shim_->is_closed()) {
    try {
      close();
    } catch (...) {
      /* pass, best effort */
    }
  }
}

void channel::TLSChannel::connect(Callbacks &callbacks)
{
  if (tls_shim_ && !tls_shim_->is_closed()) {
    throw std::runtime_error("TLSChannel: (re)connect with unclosed client");
  }

  callbacks_ = &callbacks;

  // FIXME: this should be done at TCP `on_connect`, before calling the inner
  // callback's `on_connect`.
  tls_shim_ = std::make_unique<internal::TLSShim>(*ssl_context_);

  SSL *ssl = tls_shim_->get_SSL();

  /* add a pointer to _this_ to the channel_index */
  int res = SSL_set_ex_data(ssl, Initializer::channel_index, this);
  if (res != 1) {
    LOG::error("TLSChannel: could not add this reference to index: {}", TLSError());
    callbacks_->on_error(-EFAULT);
    return;
  }

  /* Configure the expected hostname in the SNI extension */
  if (!server_hostname_.empty()) {
    int res = SSL_set_tlsext_host_name(ssl, server_hostname_.c_str());
    if (res != 1) {
      LOG::error("TLSChannel: could not set SNI field: ", TLSError());
      callbacks_->on_error(-EFAULT);
      return;
    }
  }

  /* register the certificate verification callback */
  SSL_set_verify(ssl, SSL_VERIFY_PEER, verify_callback);

  /* start connecting */
  SSL_set_connect_state(ssl);
  handshake();

  if (auto error = flush_transport_bio()) {
    callbacks_->on_error(-error.value());
    return;
  }
}

u32 channel::TLSChannel::received_data(const u8 *data, u32 data_len)
{
  ASSUME(tls_shim_);
  BIO *bio = tls_shim_->get_transport_bio();
  SSL *ssl = tls_shim_->get_SSL();

  int finished_before = SSL_is_init_finished(ssl);

  int wrote = BIO_write(bio, data, data_len);
  if (wrote < 0) {
    LOG::error("TLSChannel: failed to write received transport data: {}", TLSError());
    callbacks_->on_error(-ENOTCONN);
    return 0;
  }

  if (!finished_before) {
    int res = handshake();

    if (res != 0)
      return wrote; /* handshake() already called on_error() callback */

    if (auto error = flush_transport_bio()) {
      callbacks_->on_error(-error.value());
      return wrote;
    }
  }

  /* has the handshake finished? */
  if (SSL_is_init_finished(ssl)) {
    /* we might have data to give the application */
    u64 buf[(APP_READ_SIZE + 7) / 8];
    int read = SSL_read(ssl, buf, APP_READ_SIZE);
    if (read > 0) {
      /* read some data, pass it to the caller */
      callbacks_->received_data((u8 *)buf, read);
    } else {
      int err = ERR_get_error();
      if ((err != SSL_ERROR_WANT_READ) && (err != SSL_ERROR_WANT_WRITE) && (err != SSL_ERROR_NONE)) {
        LOG::error("TLSChannel: application-side read failed: {}", TLSError(err));
        callbacks_->on_error(-ENOTCONN);
      }
    }
  }

  return (u32)wrote;
}

void channel::TLSChannel::close()
{
  if (!tls_shim_ || tls_shim_->is_closed()) {
    LOG::trace_in(
        channel::Component::tls,
        "TLSChannel::{}: (tls_shim={}): '{}'",
        __func__,
        static_cast<bool>(tls_shim_),
        server_hostname_);
    return;
  }

  LOG::trace_in(channel::Component::tls, "TLSChannel::{}: '{}'", __func__, server_hostname_);
  SSL *ssl = tls_shim_->get_SSL();
  if (SSL_is_init_finished(ssl)) {
    int res = SSL_shutdown(ssl);
    int err = ERR_get_error();
    if ((res < 0) && (err != SSL_ERROR_WANT_READ) && (err != SSL_ERROR_WANT_WRITE)) {
      throw std::runtime_error(fmt::format("TLSChannel::{}: unexpected error in close(): {}", __func__, TLSError(err)));
    }
  }

  /* flush to the transport, but do best effort; ignore errors */
  flush_transport_bio();

  tls_shim_.reset();
}

std::error_code channel::TLSChannel::send(const u8 *data, int data_len)
{
  ASSUME(tls_shim_);
  ASSUME(is_open());
  SSL *ssl = tls_shim_->get_SSL();

  while (data_len > 0) {
    /* try to write some bytes */
    int res = SSL_write(ssl, data, data_len);

    if (res > 0) {
      /* success. move on and if more to send, send it */
      data_len -= res;
      data += res;
      continue;
    }

    /* make sure only a flush is required */
    int err = ERR_get_error();
    if ((err != SSL_ERROR_WANT_READ) && (err != SSL_ERROR_WANT_WRITE) && (err != SSL_ERROR_NONE)) {
      // TODO: gracefully handle TLS errors
      throw std::runtime_error(fmt::format("TLSChannel: application-side send failed: {}", TLSError(err)));
    }

    /* error might be due to full buffers. try to flush */
    if (auto error = flush_transport_bio()) {
      callbacks_->on_error(-error.value());
      return error;
    }

    /* try again now buffer is flushed */
    res = SSL_write(ssl, data, data_len);

    if (res > 0) {
      /* success. move on and if more to send, send it */
      data_len -= res;
      data += res;
      continue;
    } else {
      /* this shouldn't happen because we've disabled renegotiation, so
       * if the buffer is flushed, forward progress should be possible
       */
      throw std::runtime_error(fmt::format("TLSChannel: application-side send failed after flush: {}", TLSError(err)));
    }

    /* in one of the above SSL_write()'s, res returns > 0 and data_len was
     * reduced, so an infinite loop should not be possible.
     */
  }

  /* flush at the end. we don't won't to keep data around until next send */
  if (auto error = flush_transport_bio()) {
    callbacks_->on_error(-error.value());
    return error;
  }

  return {};
}

std::error_code channel::TLSChannel::flush_transport_bio()
{
  ASSUME(tls_shim_);
  BIO *bio = tls_shim_->get_transport_bio();

  int pending = BIO_pending(bio);
  ASSUME(pending >= 0);
  if (pending == 0) {
    return {};
  }

  if (transport_.is_open()) {
    auto bufp = transport_.allocate_send_buffer(pending);
    if (bufp == nullptr) {
      LOG::error("TLSChannel: memory allocation failed when flushing transport");
      return std::make_error_code(std::errc::not_enough_memory);
    }

    int read = BIO_read(bio, &bufp->data[0], pending);
    if (read != pending) {
      /* free the allocated buffer memory */
      bufp->len = 0;
      transport_.send(bufp);

      /* error */
      LOG::error("TLSChannel: could not read entire buffer when flushing transport");
      return std::make_error_code(std::errc::io_error);
    }

    bufp->len = pending;
    transport_.send(bufp);
  }
  return {};
}

int channel::TLSChannel::handshake()
{
  SSL *ssl = tls_shim_->get_SSL();

  int res = SSL_do_handshake(ssl);
  switch (res) {
  case 0:
    /* handshake failed permanently */
    LOG::error("TLSChannel: handshake failed: {}", TLSError());
    callbacks_->on_error(-EACCES);
    return -EACCES;
  case 1:
    /* finished handshake, notify the instance's user */
    callbacks_->on_connect();
    return 0;
  default: {
    int err = ERR_get_error();
    if ((err == SSL_ERROR_WANT_READ) || (err == SSL_ERROR_WANT_WRITE) || (err == SSL_ERROR_NONE)) {
      return 0; /* will need more iterations */
    }

    /* abnormal error */
    LOG::error("TLSChannel: unexpected error in handshake: {}", TLSError(err));
    callbacks_->on_error(-EFAULT);
    return -EFAULT;
  }
  }
}

const std::string &channel::TLSChannel::peer_hostname()
{
  return server_hostname_;
}
