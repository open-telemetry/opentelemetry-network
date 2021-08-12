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

#include <util/restful.h>

RestfulFetcher::RestfulFetcher(std::initializer_list<std::string_view> headers)
{
  for (auto const header : headers) {
    headers_.emplace_back(header);
  }
}

void RestfulFetcher::http_proxy(std::string host, std::uint16_t port)
{
  proxy_ = std::move(host);
  proxy_port_ = port;
  proxy_type_ = CURLPROXY_HTTP;
}
