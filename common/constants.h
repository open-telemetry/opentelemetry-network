/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include <util/preprocessor.h>
#include <util/version.h>

#include <string>

namespace kernel {
constexpr u16 MIN_CGROUP_CPU_SHARES = 2;
constexpr u16 MAX_CGROUP_CPU_SHARES = 1024;
constexpr u32 DEFAULT_CGROUP_QUOTA = 100'000;
} // namespace kernel

namespace versions {

constexpr VersionInfo release{FLOWMILL_MAJOR_VERSION, FLOWMILL_MINOR_VERSION, FLOWMILL_COLLECTOR_BUILD_NUMBER};

} // namespace versions
