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

#include <channel/internal/ssl_context.h>
#include <channel/tls_error.h>
#include <exception>
#include <openssl/err.h>

#include <util/log.h>

static const char *CIPHER_LIST = "ECDHE-RSA-AES256-GCM-SHA384";

channel::internal::SSLContext::SSLContext(const std::string &key, const std::string &cert)
{
  if (!key.empty()) {
    private_key_.emplace(key);
  }
  if (!cert.empty()) {
    client_cert_.emplace(cert);
  }

  const SSL_METHOD *method = TLS_client_method();
  if (method == NULL) {
    throw std::runtime_error(fmt::format("could not initialize TLS method: {}", TLSError()));
  }

  ctx_ = SSL_CTX_new(method);
  if (ctx_ == nullptr) {
    throw std::runtime_error(fmt::format("could not make new TLS context: {}", TLSError()));
  }

  try {
    /**
     * - remove compression
     * - remove renegotiation so calls to SSL_write will fail only if
     *   buffer space is required (and not because multiple round trips
     *   are required to renegotiate. this simlplifies state machine
     */
    long old_opts = SSL_CTX_set_options(ctx_, SSL_OP_NO_COMPRESSION | SSL_OP_NO_RENEGOTIATION);
    (void)(old_opts); /* unused */

    /* we only support TLS1.2 and above */
    int res = SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);
    if (res != 1) {
      throw std::runtime_error(fmt::format("could not set minimum TLS version: {}", TLSError()));
    }

    res = SSL_CTX_set_cipher_list(ctx_, CIPHER_LIST);
    if (res != 1) {
      throw std::runtime_error(fmt::format("Could not select TLS cipher: {}", TLSError()));
    }

    /**
     * Use CA certificates defined from the host
     * This needs to be `SSL_CTX_set_default_verify_paths`
     * because `SSL_CTX_set_default_verify_dir` was not picking up
     * the SSL_CERT_DIR or SSL_CERT_FILE environment variables,
     * which are important for configuring OpenSSL across different distros
     *
     * For more info see:
     * https://www.happyassassin.net/2015/01/12/a-note-about-ssltls-trusted-certificate-stores-and-platforms/
     */
    res = SSL_CTX_set_default_verify_paths(ctx_);
    if (res != 1) {
      throw std::runtime_error(fmt::format("Could not set host certificate authorities", TLSError()));
    }

    /**
     * Client certificates
     */
    if (client_cert_.has_value()) {
      LOG::debug("using a client certificate");
      res = SSL_CTX_use_certificate(ctx_, client_cert_->get());
      if (res != 1) {
        throw std::runtime_error(fmt::format("Could not configure TLS client cert: {}", TLSError()));
      }
    }

    if (private_key_.has_value()) {
      LOG::debug("using a client private key");
      res = SSL_CTX_use_PrivateKey(ctx_, private_key_->get());
      if (res != 1) {
        throw std::runtime_error(fmt::format("Could not configure TLS private key: {}", TLSError()));
      }

      res = SSL_CTX_check_private_key(ctx_);
      if (res != 1) {
        throw std::runtime_error(fmt::format("TLS private key check failed: {}", TLSError()));
      }
    }

    long old_mode = SSL_CTX_set_mode(ctx_, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    (void)old_mode; /* unused */
  } catch (std::exception &e) {
    SSL_CTX_free(ctx_);
    throw;
  }
}

channel::internal::SSLContext::~SSLContext()
{
  SSL_CTX_free(ctx_);
}
