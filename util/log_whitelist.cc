// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
