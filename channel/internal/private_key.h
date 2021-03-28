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

#include <openssl/evp.h>
#include <string>

namespace channel {
namespace internal {

class PrivateKey {
public:
  /**
   * C'tor
   * @param key: the RSA private key in text
   */
  PrivateKey(const std::string &key);

  /**
   * D'tor
   */
  ~PrivateKey();

  /**
   * Accessor.
   */
  EVP_PKEY *get() { return pkey_; }

private:
  EVP_PKEY *pkey_;
};

} /* namespace internal */
} /* namespace channel */
