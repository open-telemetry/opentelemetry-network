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

#include <util/code_timing.h>

#include <filesystem>

#if ENABLE_CODE_TIMING

CodeTiming::CodeTiming(std::string const &name, std::string const &filename, int line)
    : name_(name), filename_(std::filesystem::path(filename).filename().string()), line_(line)
{
  code_timing_registry_.register_code_timing(std::string(name + ":" + filename_ + ":" + std::to_string(line)), this);
}

void CodeTiming::set(u64 duration_ns)
{
  gauge_ += duration_ns;
}

void CodeTiming::write_stats(std::stringstream &ss, u64 timestamp, std::string const &common_labels)
{
  if (!gauge_.count()) {
    return;
  }

  ss << "codetiming_count"
     << "{name=\"" << name_ << "\",filename=\"" << filename_ << "\",line=" << line_ << "," << common_labels << "} "
     << gauge_.count() << " " << timestamp << "\n";
  ss << "codetiming_avg_ns"
     << "{name=\"" << name_ << "\",filename=\"" << filename_ << "\",line=" << line_ << "," << common_labels << "} "
     << gauge_.average<u64>() << " " << timestamp << "\n";
  ss << "codetiming_min_ns"
     << "{name=\"" << name_ << "\",filename=\"" << filename_ << "\",line=" << line_ << "," << common_labels << "} "
     << gauge_.min() << " " << timestamp << "\n";
  ss << "codetiming_max_ns"
     << "{name=\"" << name_ << "\",filename=\"" << filename_ << "\",line=" << line_ << "," << common_labels << "} "
     << gauge_.max() << " " << timestamp << "\n";
  ss << "codetiming_sum_ns"
     << "{name=\"" << name_ << "\",filename=\"" << filename_ << "\",line=" << line_ << "," << common_labels << "} "
     << gauge_.sum() << " " << timestamp << "\n";

  gauge_.reset();
}

std::ostream &operator<<(std::ostream &os, CodeTiming const &timing)
{
  os << "{"
     << "count=" << timing.gauge_.count() << " avg=" << timing.gauge_.average<u64>() << " min=" << timing.gauge_.min()
     << " max=" << timing.gauge_.max() << " sum=" << timing.gauge_.sum() << '}';

  return os;
}

thread_local CodeTimingRegistry code_timing_registry_;

void CodeTimingRegistry::register_code_timing(std::string const &name, CodeTiming *timing)
{
  code_timings_[name] = timing;
}

void CodeTimingRegistry::write_stats(std::stringstream &ss, u64 timestamp, std::string const &common_labels)
{
  for (auto const &[name, timing] : code_timings_) {
    timing->write_stats(ss, timestamp, common_labels);
  }
}

#endif // ENABLE_CODE_TIMING
