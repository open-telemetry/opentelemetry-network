// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/resource_usage_reporter.h>

#include <util/log.h>
#include <util/system_ops.h>

#include <sys/resource.h>
#include <sys/time.h>

#include <chrono>
#include <utility>

using namespace std::literals::chrono_literals;

constexpr auto REPORT_INTERVAL = 1s;

ResourceUsageReporter::ResourceUsageReporter(uv_loop_t &loop, ::flowmill::ingest::Writer &writer)
    : last_check_(monotonic_clock::now()), writer_(writer), scheduler_(loop, [this] {
        this->collect();
        return scheduling::JobFollowUp::ok;
      })
{}

void ResourceUsageReporter::start()
{
  if (!scheduler_.start(REPORT_INTERVAL, REPORT_INTERVAL)) {
    LOG::error("failed to schedule resource usage reporter");
  }
}

void ResourceUsageReporter::stop()
{
  scheduler_.stop();
}

void ResourceUsageReporter::collect()
{
  if (auto const info = get_resource_usage()) {
    auto const now = monotonic_clock::now();
    auto const interval = now - last_check_;
    auto const cpu_time = ResourceUsageReporter::duration{info->kernel_mode_time + info->user_mode_time};
    // per-thousand fixed point, lowest 3 digits are fractional
    auto const cpu_perthousand = (cpu_time * 1000) / interval;

    writer_.agent_resource_usage(
        integer_time<std::chrono::microseconds>(info->user_mode_time),
        integer_time<std::chrono::microseconds>(info->kernel_mode_time),
        info->max_resident_set_size,
        info->minor_page_faults,
        info->major_page_faults,
        info->block_input_count,
        info->block_output_count,
        info->voluntary_context_switch_count,
        info->involuntary_context_switch_count,
        cpu_perthousand,
        0 // TODO: properly gather this value
    );

    last_check_ = now;
  } else {
    LOG::error("failed to query resource usage: {}", info.error().what());
  }
}

void ResourceUsageReporter::report(::flowmill::ingest::Writer &writer)
{
  if (auto const info = get_resource_usage()) {
    writer.agent_resource_usage(
        integer_time<std::chrono::microseconds>(info->user_mode_time),
        integer_time<std::chrono::microseconds>(info->kernel_mode_time),
        info->max_resident_set_size,
        info->minor_page_faults,
        info->major_page_faults,
        info->block_input_count,
        info->block_output_count,
        info->voluntary_context_switch_count,
        info->involuntary_context_switch_count,
        0,
        0 // TODO: properly gather this value
    );
  } else {
    LOG::error("failed to query resource usage: {}", info.error().what());
  }
}
