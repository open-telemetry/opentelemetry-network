// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/troubleshooting.h>

#include <common/linux_distro.h>
#include <util/log_formatters.h>

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>

#include <iostream>
#include <string_view>
#include <thread>

#include <cstdlib>

static constexpr auto EXIT_SLEEP_GRACE_PERIOD_DEFAULT = 60s;
static constexpr auto EXIT_SLEEP_FOREVER = std::chrono::seconds::max();

void close_agent(int exit_code, std::function<void()> flush_and_close, std::chrono::seconds exit_sleep_sec)
{
  if (flush_and_close) {
    flush_and_close();
  }
#if NDEBUG // delay exit for release builds but not for debug builds to prevent test failures from blocking
  std::this_thread::sleep_for(exit_sleep_sec);
#endif
  exit(exit_code);
}

void print_troubleshooting_message_and_exit(
    HostInfo const &info,
    TroubleshootItem item,
    std::exception const &e,
    std::optional<std::reference_wrapper<logging::Logger>> log,
    std::function<void()> flush_and_close)
{
  if (item == TroubleshootItem::none) {
    return;
  }

  auto const item_name = to_string(item);
  auto const os_name = to_string(info.os);
  auto const distro_name = to_string(static_cast<LinuxDistro>(info.os_flavor));
  auto const headers_source = to_string(info.kernel_headers_source);
  auto const exception_message = std::string(e.what());

  auto const item_fmt = fmt::format(
      "troubleshoot item {} (os={},flavor={},headers_src={},kernel={}): {}",
      item_name,
      os_name,
      distro_name,
      headers_source,
      info.kernel_version,
      exception_message);

  if (log) {
    log->get().error(item_fmt);
  } else {
    LOG::error(item_fmt);
  }

  auto exit_sleep_sec = EXIT_SLEEP_GRACE_PERIOD_DEFAULT;
  switch (item) {

  case TroubleshootItem::bpf_load_probes_failed: {
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Failed to load eBPF probes for the Linux distro '{}' running kernel version {}.

{}

)TROUBLESHOOTING",
        distro_name,
        info.kernel_version,
        item_fmt);
    break;
  }

  case TroubleshootItem::operation_not_permitted: {
    exit_sleep_sec = EXIT_SLEEP_FOREVER;
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Insufficient permissions to perform a priviliged operation.
Priviliged operations include mounting debugfs, loading eBPF code and probes, etc.
Make sure to run as privileged user, and/or with --privileged flag or equivalant.

{}

Blocking to avoid failure retry loop.

)TROUBLESHOOTING",
        item_fmt);
    break;
  }

  case TroubleshootItem::permission_denied: {
    exit_sleep_sec = EXIT_SLEEP_FOREVER;
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Operation failed with permission denied.
This can occur due to SELinux policy enforcement.  If SELinux is enabled, make sure to first run
the provided script to apply an SELinux policy allowing eBPF operations.

{}

Blocking to avoid failure retry loop.

)TROUBLESHOOTING",
        item_fmt);
    break;
  }

  case TroubleshootItem::unexpected_exception: {
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Unexpected exception.

{}

)TROUBLESHOOTING",
        item_fmt);
    break;
  }

  default: {
    std::cout << R"TROUBLESHOOTING(
Unknown error happened in the Kernel Collector.

)TROUBLESHOOTING";
    break;
  }
  }

  std::cout << std::endl;
  std::cout.flush();

  close_agent(-1, flush_and_close, exit_sleep_sec);
}
