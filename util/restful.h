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

#include <platform/types.h>
#include <util/expected.h>

#include <curl/curl.h>

#include <chrono>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <string>
#include <string_view>

class RestfulFetcher {
public:
  template <typename T> class CtorDecoder {
  public:
    Expected<T, std::runtime_error> operator()(std::string body) const;
  };

  RestfulFetcher(std::initializer_list<std::string_view> headers = {});

  void http_proxy(std::string host, std::uint16_t port);

  template <typename T, typename Decoder = CtorDecoder<T>>
  Expected<T, std::runtime_error> sync_fetch(
      std::string_view description,
      std::string url,
      Decoder &&decoder = Decoder(),
      std::chrono::milliseconds const timeout = 0ms,
      std::size_t const retries = 0,
      std::chrono::milliseconds const initial_backoff = 200ms,
      std::chrono::milliseconds const maximum_backoff = 1min,
      unsigned const backoff_geometric_ratio = 2);

private:
  std::list<std::string> headers_;
  std::string proxy_;
  long proxy_port_;
  curl_proxytype proxy_type_;
};

#include <util/restful.inl>
