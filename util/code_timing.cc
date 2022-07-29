// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/code_timing.h>
#include <util/log.h>

#include <filesystem>

#if ENABLE_CODE_TIMING

u64 CodeTiming::next_index_ = 0;

CodeTiming::CodeTiming(std::string const &name, std::string const &filename, int line)
    : name_(name),
      filename_(std::filesystem::path(filename).filename().string()),
      line_(line),
      index_(next_index_++),
      full_name_(fmt::format("{}:{}:{}:{}", name_, filename_, line_, index_))
{
  code_timing_registry_.register_code_timing(full_name_, this);
}

CodeTiming::~CodeTiming()
{
  code_timing_registry_.unregister_code_timing(full_name_);
}

void CodeTiming::set(u64 duration_ns)
{
  gauge_ += duration_ns;
}

void CodeTiming::visit(VisitCallback func)
{
  func(name_, filename_, line_, index_, gauge_);
}

void CodeTiming::print()
{
  LOG::info("  {} [{}:{} ({})] {}", name_, filename_, line_, index_, *this);
}

std::ostream &operator<<(std::ostream &os, CodeTiming const &timing)
{
  os << "{"
     << "count=" << timing.gauge_.count() << " avg=" << timing.gauge_.average<u64>() << " min=" << timing.gauge_.min()
     << " max=" << timing.gauge_.max() << " sum=" << timing.gauge_.sum() << '}';

  return os;
}

thread_local CodeTimingRegistry code_timing_registry_;

void print_code_timings()
{
  code_timing_registry_.print();
}

void CodeTimingRegistry::register_code_timing(std::string const &name, CodeTiming *timing)
{
  code_timings_[name] = timing;
}

void CodeTimingRegistry::unregister_code_timing(std::string const &name)
{
  code_timings_.erase(name);
}

void CodeTimingRegistry::visit(CodeTiming::VisitCallback func)
{
  for (auto const &[name, timing] : code_timings_) {
    timing->visit(func);
  }
}

void CodeTimingRegistry::print()
{
  LOG::info("CodeTimings for thread id hash=0x{:x}:", std::hash<std::thread::id>{}(std::this_thread::get_id()));
  for (auto const &[name, timing] : code_timings_) {
    timing->print();
  }
}

#endif // ENABLE_CODE_TIMING
