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
