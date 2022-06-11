//
// Copyright 2022 Splunk Inc.
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

#include <util/defer.h>
#include <util/gauge.h>
#include <util/preprocessor.h>
#include <util/stop_watch.h>

#include <map>
#include <string>

#if ENABLE_CODE_TIMING

class CodeTiming {
  friend std::ostream &operator<<(std::ostream &os, CodeTiming const &timing);

public:
  CodeTiming(std::string const &name, std::string const &filename, int line);
  ~CodeTiming();

  void set(u64 duration_ns);

  void write_stats(std::stringstream &ss, u64 timestamp, std::string const &common_labels = "");

  void print();

private:
  const std::string name_;
  const std::string filename_;
  const int line_;
  data::Gauge<u64> gauge_;

  std::string full_name();
};

class CodeTimingRegistry {
public:
  // Register a CodeTiming.
  void register_code_timing(std::string const &name, CodeTiming *timing);

  // Unregister a CodeTiming.
  void unregister_code_timing(std::string const &name);

  // Writes all registered code timings out as internal stats.
  void write_stats(std::stringstream &ss, u64 timestamp, std::string const &common_labels = "");

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
