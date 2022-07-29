// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/url.h>

std::string format_url(std::string host, std::string_view path, std::string_view default_scheme)
{
  if (host.empty()) {
    return host;
  }

  if (host.find("://") == std::string::npos) {
    host.insert(0, "://");
    host.insert(0, default_scheme);
  }

  if (host.back() != '/' && !path.empty() && path.front() != '/') {
    host.push_back('/');
  }

  host.append(path);

  return host;
}
