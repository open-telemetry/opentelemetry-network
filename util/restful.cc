// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
