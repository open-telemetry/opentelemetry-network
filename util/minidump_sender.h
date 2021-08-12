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

#include <list>
#include <map>
#include <string>

/**
 * Sends a minidump file to the server.
 */
class MinidumpSender {
public:
  /**
   * Sends the minidump to the server
   *
   * Based on prototype of google_breakpad::HTTPUpload::SendRequest
   */
  bool send(
      std::string const &url,
      std::list<std::string> headers,
      std::map<std::string, std::string> const &parameters,
      std::map<std::string, std::string> const &files,
      std::string const &proxy = {},
      std::string const &proxy_user_pwd = {},
      std::string const &ca_certificate_file = {});

  std::string const &response_body() const { return response_body_; }
  long response_code() const { return response_code_; }
  std::string const &error_description() const { return error_description_; }

private:
  std::string response_body_;
  long response_code_ = 0;
  std::string error_description_;
};
