/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include <util/defer.h>
#include <util/gauge.h>
#include <util/preprocessor.h>
#include <util/stop_watch.h>

#include <map>
#include <string>
#include <sstream>
#include <spdlog/fmt/fmt.h>

#if ENABLE_CODE_TIMING

class CodeTiming {
  friend std::ostream &operator<<(std::ostream &os, CodeTiming const &timing);

public:
  using VisitCallback = std::function<void(std::string_view, std::string_view, int, u64, data::Gauge<u64> &)>;

  CodeTiming(std::string const &name, std::string const &filename, int line);
  ~CodeTiming();

  void set(u64 duration_ns);

  void visit(VisitCallback func);

  void print();

private:
  const std::string name_;
  const std::string filename_;
  const int line_;
  static u64 next_index_;
  const u64 index_; // to make sure CodeTimings in templated functions have a unique key for the CodeTimingRegistry
  const std::string full_name_;
  data::Gauge<u64> gauge_;
};

class CodeTimingRegistry {
public:
  // Register a CodeTiming.
  void register_code_timing(std::string const &name, CodeTiming *timing);

  // Unregister a CodeTiming.
  void unregister_code_timing(std::string const &name);

  void visit(CodeTiming::VisitCallback func);

  // Prints all registered code timings out via LOG::info().
  void print();

private:
  std::map<std::string, CodeTiming *> code_timings_;
};

extern thread_local CodeTimingRegistry code_timing_registry_;
extern void print_code_timings();

#define SCOPED_TIMING(name)                                                                                                    \
  static thread_local CodeTiming timing_##name(#name, __FILE__, __LINE__);                                                     \
  StopWatch sw_##name;                                                                                                         \
  DEFER([&] { timing_##name.set(sw_##name.elapsed_ns()); });

#define START_TIMING(name)                                                                                                     \
  static thread_local CodeTiming timing_##name(#name, __FILE__, __LINE__);                                                     \
  StopWatch sw_##name;

#define STOP_TIMING(name) timing_##name.set(sw_##name.elapsed_ns());

#else // ENABLE_CODE_TIMING

#define SCOPED_TIMING(name)
#define START_TIMING(name)
#define STOP_TIMING(name)

#endif // ENABLE_CODE_TIMING

// Provide fmt support for CodeTiming using its ostream representation when enabled.
#if ENABLE_CODE_TIMING
namespace fmt {
template <> struct formatter<CodeTiming> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(CodeTiming const &ct, FormatContext &ctx) const
  {
    std::ostringstream os;
    os << ct;
    return fmt::format_to(ctx.out(), "{}", os.str());
  }
};
} // namespace fmt
#endif
