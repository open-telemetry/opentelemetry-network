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

#include <platform/platform.h>

class KernelCollector;

/**
 * Helper class to facilitate restarting KernelCollector, i.e. when BufferedPoller detects lost BPF samples (PERF_RECORD_LOST).
 * If KernelCollector startup has completed, then a restart request will be processed immediately.
 * If KernelCollector startup has not completed, then the processing of a restart request will be deferred until after
 * KernelCollector startup has completed.
 * If a restart is already in progress, subsequent restart requests will be ignored.
 */
class KernelCollectorRestarter {
  friend class KernelCollector;

public:
  KernelCollectorRestarter(KernelCollector &collector);
  void startup_completed();
  void request_restart();
  void reset();

private:
  void check_restart();

  bool startup_completed_;
  bool restart_requested_;
  bool restart_in_progress_;

  KernelCollector &collector_;
};
