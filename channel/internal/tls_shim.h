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

#include <memory>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <string>

namespace channel {
namespace internal {

class SSLContext;

/**
 * A class that maintains the objects for an OpenSSL encrypting shim (gets TLS
 *   data from TCP, decrypts and passes to the application, and back)
 */
class TLSShim {
public:
  /**
   * C'tor
   * @param key: the client (private) key in PEM
   * @param cert: the client certificate, in PEM
   */
  TLSShim(SSLContext &ctx);

  /**
   * D'tor
   */
  ~TLSShim();

  /**
   * @returns true if the TLS connection is shutdown, false otherwise
   */
  bool is_closed();

  /**
   * Get SSL object
   */
  SSL *get_SSL() { return ssl_; }

  /**
   * Get tcp-facing BIO
   */
  BIO *get_transport_bio() { return transport_bio_; }

private:
  SSL *ssl_;
  BIO *transport_bio_;
};

} /* namespace internal */
} /* namespace channel */
