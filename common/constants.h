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
