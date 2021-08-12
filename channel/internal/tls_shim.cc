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
#include <channel/internal/tls_shim.h>
#include <channel/tls_error.h>
#include <util/log.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

channel::internal::TLSShim::TLSShim(SSLContext &ctx) : ssl_(nullptr), transport_bio_(nullptr)
{
  ssl_ = SSL_new(ctx.get());
  if (ssl_ == nullptr) {
    throw std::runtime_error(fmt::format("could not instantiate TLS object: {}", TLSError()));
  }

  BIO *ssl_obj_bio = nullptr;
  int res = BIO_new_bio_pair(&transport_bio_, 0, &ssl_obj_bio, 0);
  if (res != 1) {
    SSL_free(ssl_);
    ssl_ = nullptr;
    throw std::runtime_error(fmt::format("could not allocate TLS bio pair: {}", TLSError()));
  }

  /* we will pass two references of ssl_obj_bio_ to ssl_: read and write */
  BIO_up_ref(ssl_obj_bio);
  SSL_set0_rbio(ssl_, ssl_obj_bio);
  SSL_set0_wbio(ssl_, ssl_obj_bio);
  /* from this point onwards, ssl_ will free ssl_obj_bio_ */
}

channel::internal::TLSShim::~TLSShim()
{
  /* free ssl_, this will also free ssl_obj_bio */
  SSL_free(ssl_);
  ssl_ = nullptr;

  /* free transport_bio_ */
  BIO_free(transport_bio_);
  transport_bio_ = nullptr;
}

bool channel::internal::TLSShim::is_closed()
{
  return (SSL_is_init_finished(ssl_) == 0 || SSL_get_shutdown(ssl_) != 0);
}
