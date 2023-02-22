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

constexpr std::string_view KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS = R"INSTRUCTIONS(
In the meantime, please install kernel headers manually on each host before running
the Kernel Collector.

To manually install kernel headers, follow the instructions below:

  - for Debian/Ubuntu based distros, run:

      sudo apt-get install --yes "linux-headers-`uname -r`"

  - for RedHat based distros like CentOS and Amazon Linux, run:

      sudo yum install -y "kernel-devel-`uname -r`"
)INSTRUCTIONS";

void close_agent(int exit_code, std::function<void()> flush_and_close, std::chrono::seconds exit_sleep_sec)
{
  if (flush_and_close) {
    flush_and_close();
  }
  std::this_thread::sleep_for(exit_sleep_sec);
  exit(exit_code);
}

void print_troubleshooting_message_and_exit(
    HostInfo const &info, EntrypointError error, logging::Logger &log, std::function<void()> flush_and_close)
{
  if (error == EntrypointError::none) {
    return;
  }

  log.error(
      "entrypoint error {} (os={},flavor={},headers_src={},kernel={})",
      error,
      info.os,
      info.os_flavor,
      info.kernel_headers_source,
      info.kernel_version);

  switch (error) {
  case EntrypointError::unsupported_distro: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
Automatically fetching kernel headers for the Linux distro '{}' is currently unsupported.

We're regularly adding kernel headers fetching support for popular Linux distros so if
you're using a well known distro, please reach out to support so we can better support
your use case.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor))
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  case EntrypointError::kernel_headers_fetch_error: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
Problem while installing kernel headers for the host's Linux distro '{}'.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor))
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  case EntrypointError::kernel_headers_fetch_refuse: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
As requested, refusing to automatically fetch kernel headers for the hosts's Linux distro '{}'.

In order to allow it, follow the instructions below:

  - for deployments using the helm charts from https://github.com/signalfx/splunk-otel-collector-chart,
    set `kernelCollector.installKernelHeaders` to `true` in `values.yaml`:

      kernelCollector:
        installKernelHeaders: true

  - for deployments using Docker, set environment variable `EBPF_NET_KERNEL_HEADERS_AUTO_FETCH`
    to `true` by passing these additional command line flags to Docker:

      --env "EBPF_NET_KERNEL_HEADERS_AUTO_FETCH=true"

  - for ECS deployments, ensure that environment variable `EBPF_NET_KERNEL_HEADERS_AUTO_FETCH`
    is set to `true` in the Agent's task definitions;

  - for any other deployments, ensure that the Kernel Collector container will be started
    with environment variable `EBPF_NET_KERNEL_HEADERS_AUTO_FETCH` set to `true`.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor))
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  case EntrypointError::kernel_headers_missing_repo: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
Unable to locate the configuration for the host's package manager in order to automatically
install kernel headers for the Linux distro '{}'.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor))
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  case EntrypointError::kernel_headers_misconfigured_repo: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
Unable to use the host's package manager configuration to automatically install kernel headers
for the Linux distro '{}'.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor))
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  default: {
    std::cout << R"TROUBLESHOOTING(
Unknown error happened during boot up of the Kernel Collector.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING";
    break;
  }
  }

  std::cout << std::endl;
  std::cout.flush();

  if (flush_and_close) {
    flush_and_close();
  }
  close_agent(-1, flush_and_close, EXIT_SLEEP_GRACE_PERIOD_DEFAULT);
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

  auto const item_fmt = fmt::format(
      "troubleshoot item {} (os={},flavor={},headers_src={},kernel={}): {}",
      item,
      info.os,
      static_cast<LinuxDistro>(info.os_flavor),
      info.kernel_headers_source,
      info.kernel_version,
      e);

  if (log) {
    log->get().error(item_fmt);
  } else {
    LOG::error(item_fmt);
  }

  auto exit_sleep_sec = EXIT_SLEEP_GRACE_PERIOD_DEFAULT;
  switch (item) {
  case TroubleshootItem::bpf_compilation_failed: {
    std::cout << fmt::format(
                     R"TROUBLESHOOTING(
Failed to compile eBPF code for the Linux distro '{}' running kernel version {}.

{}

This usually means that kernel headers weren't installed correctly.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
                     static_cast<LinuxDistro>(info.os_flavor),
                     info.kernel_version,
                     item_fmt)
              << std::endl
              << KERNEL_HEADERS_MANUAL_INSTALL_INSTRUCTIONS;
    break;
  }

  case TroubleshootItem::bpf_load_probes_failed: {
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Failed to load eBPF probes for the Linux distro '{}' running kernel version {}.

{}

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
        static_cast<LinuxDistro>(info.os_flavor),
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

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
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

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
        item_fmt);
    break;
  }

  case TroubleshootItem::unexpected_exception: {
    std::cout << fmt::format(
        R"TROUBLESHOOTING(
Unexpected exception.

{}

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING",
        item_fmt);
    break;
  }

  default: {
    std::cout << R"TROUBLESHOOTING(
Unknown error happened in the Kernel Collector.

Please reach out to support and include this log in its entirety so we can diagnose and fix
the problem.
)TROUBLESHOOTING";
    break;
  }
  }

  std::cout << std::endl;
  std::cout.flush();

  close_agent(-1, flush_and_close, exit_sleep_sec);
}
