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

#include <memory>

#include <collector/kernel/perf_reader.h>
#include <platform/platform.h>

class PerfPoller {
public:
  /**
   * c'tor
   */
  PerfPoller(PerfContainer &container);

  /**
   * d'tor
   */
  virtual ~PerfPoller();

  /**
   * poll method, to be implemented by child class.
   *
   * Can use container_->...
   */
  virtual void poll() = 0;

  /**
   * Perform @n_intervals pollings, every @interval_useconds microseconds
   */
  void start(u64 interval_useconds, u64 n_intervals);

protected:
  PerfContainer &container_;
};
