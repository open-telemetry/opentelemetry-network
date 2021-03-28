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

#include <util/time.h>

#include <sys/time.h>
#include <unistd.h>

#include <stdexcept>
#include <system_error>

#include <cerrno>

std::uint64_t const clock_ticks_per_second = [] {
  if (auto const result = sysconf(_SC_CLK_TCK); result != -1) {
    return static_cast<std::uint64_t>(result);
  }

  throw std::system_error{errno, std::generic_category()};
}();
