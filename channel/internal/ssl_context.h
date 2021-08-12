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

#pragma once

#include <channel/internal/certificate.h>
#include <channel/internal/private_key.h>
#include <memory>
#include <openssl/ssl.h>
#include <optional>
#include <string>

namespace channel {
namespace internal {

class SSLContext {
public:
  /**
   * C'tor
   * @param key: the client (private) key in PEM
   * @param cert: the client certificate, in PEM
   */
  SSLContext(const std::string &key, const std::string &cert);

  /**
   * D'tor
   */
  ~SSLContext();

  /**
   * Accessor.
   */
  SSL_CTX *get() { return ctx_; }

private:
  std::optional<channel::internal::PrivateKey> private_key_;
  std::optional<channel::internal::Certificate> client_cert_;
  SSL_CTX *ctx_;
};

} /* namespace internal */
} /* namespace channel */
