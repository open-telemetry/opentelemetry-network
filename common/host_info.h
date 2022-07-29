/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/kernel_headers_source.h>
#include <common/linux_distro.h>
#include <common/operating_system.h>

#include <cstdint>

struct HostInfo {
  OperatingSystem const os;
  std::uint8_t const os_flavor;
  std::string const os_version;
  KernelHeadersSource const kernel_headers_source;
  std::string const kernel_version;
  std::string const hostname;
};
