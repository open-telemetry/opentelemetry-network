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

#include <scheduling/interval_scheduler.h>
#include <util/resource_usage.h>
#include <util/time.h>

#include <generated/flowmill/ingest/writer.h>

class ResourceUsageReporter {
public:
  using duration = std::common_type_t<monotonic_clock::duration, ResourceUsage::duration>;

  ResourceUsageReporter(uv_loop_t &loop, ::flowmill::ingest::Writer &writer);

  void start();
  void stop();

  static void report(::flowmill::ingest::Writer &writer);

private:
  void collect();

  monotonic_clock::time_point last_check_;
  ::flowmill::ingest::Writer &writer_;
  scheduling::IntervalScheduler scheduler_;
};
