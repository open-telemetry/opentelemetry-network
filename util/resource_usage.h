/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
