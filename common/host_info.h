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
