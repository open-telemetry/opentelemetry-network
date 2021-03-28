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

#include <chrono>

#include <cstdint>

struct ResourceUsage {
  using duration = std::chrono::microseconds;

  duration user_mode_time;
  duration kernel_mode_time;
  std::uint64_t max_resident_set_size;
  std::uint32_t minor_page_faults;
  std::uint32_t major_page_faults;
  std::uint32_t block_input_count;
  std::uint32_t block_output_count;
  std::uint32_t voluntary_context_switch_count;
  std::uint32_t involuntary_context_switch_count;
  std::uint16_t cpu_usage_by_process; // per-thousand fixed point, lowest 3 digits are fractional
};
