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

#pragma once

// This file contains helper functions for system-level operations
// like gathering information about the system.

#include <util/expected.h>
#include <util/resource_usage.h>

#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <string>
#include <string_view>
#include <system_error>

static constexpr std::size_t MAX_HOSTNAME_LENGTH = 256;

// Retrieves information about the running system, as per `sysinfo(2)`.
//
// Returns the populated `sysinfo` structure on success, or the error
// information on failure.
Expected<struct ::sysinfo, std::system_error> system_info();

// Retrieves information about the host, as per `uname(2)`.
//
// Returns the populated `utsname` structure on success, or the error
// information on failure.
// Note: the member `cpu_usage_by_process` will be set to 0. To correctly
// get a time-series of cpu usage, take snapshots and compute the
// difference of user+system time, divided by the interval between the
// snapshots.
Expected<ResourceUsage, std::system_error> get_resource_usage();

// Retrieves information about the host, as per `uname(2)`.
//
// Returns the populated `utsname` structure on success, or the error
// information on failure.
Expected<struct ::utsname, std::system_error> get_host_info();

// Retrieves this host's hostname as per `uname(2)`'s `nodename`.
//
// If `max_length` is greater than 0 then the hostname will be truncated if it
// exceeds this size.
//
// On failure the error information is returned.
Expected<std::string, std::error_code> get_host_name(std::size_t max_length = 0);

// Retrieves the system-wide memory page size.
//
// On failure the error information is returned.
Expected<std::size_t, std::error_code> memory_page_size();
