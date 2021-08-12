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

#include <string_view>

namespace channel {

/**
 * A class representing a TLS error code.
 *
 * Using this class in log statements will provide properly formatted string
 * representation of TLS erros, with lazy evaluation (translation of error code
 * to string representation won't take place unless the log would be printed
 * due to log level check).
 */
class TLSError {
public:
  /**
   * Represents the TLS error code given by `ERR_get_error`.
   * @param code: an OpenSSL error code
   */
  TLSError();

  /**
   * Represents the given TLS error code.
   * @param code: an OpenSSL error code
   */
  TLSError(int code);

  int code() const { return code_; }

  std::string_view name() const;
  std::string_view reason() const;

private:
  int code_;
};

template <typename Out> Out &operator<<(Out &&out, TLSError const &error)
{
  out << "[TLS " << error.name() << ':' << error.code() << ": '" << error.reason() << "']";
  return out;
}

} /* namespace channel */
