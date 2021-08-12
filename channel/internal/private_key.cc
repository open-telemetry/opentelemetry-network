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

#include <channel/internal/private_key.h>
#include <channel/tls_error.h>
#include <util/log.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

channel::internal::PrivateKey::PrivateKey(const std::string &key)
{
  BIO *bio = BIO_new_mem_buf(key.c_str(), key.length());
  if (bio == nullptr) {
    throw std::runtime_error(fmt::format("could not allocate BIO for TLS private key: {}", TLSError()));
  }

  pkey_ = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);

  /* get error, so it doesn't get overriden by BIO_free */
  int err = ERR_get_error();
  /* free the bio. we need to free it regardless of success in reading */
  BIO_free(bio);

  /* now check success. */
  if (pkey_ == nullptr) {
    throw std::runtime_error(fmt::format("could not read TLS private key: {}", TLSError(err)));
  }
}

channel::internal::PrivateKey::~PrivateKey()
{
  EVP_PKEY_free(pkey_);
}
