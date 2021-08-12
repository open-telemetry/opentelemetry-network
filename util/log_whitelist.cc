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

#include <util/log_whitelist.h>

LogWhitelistRegistry::Proxy::Proxy(void (*whitelist_all)()) : whitelist_all(whitelist_all) {}

LogWhitelistRegistry::Proxy *LogWhitelistRegistry::head_ = nullptr;

void LogWhitelistRegistry::register_log_whitelist(Proxy &proxy)
{
  proxy.next = head_;
  head_ = &proxy;
}

void LogWhitelistRegistry::whitelist_all()
{
  for (auto proxy = head_; proxy; proxy = proxy->next) {
    proxy->whitelist_all();
  }
}

void log_whitelist_all_globally()
{
  LogWhitelistRegistry::whitelist_all();
}
