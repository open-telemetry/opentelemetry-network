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

#include <functional>
#include <memory>

class ProbeHandler;
namespace ebpf {
class BPFModule;
}

class ProcessProber {
public:
  ProcessProber(
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb);

private:
  /**
   * Iterates over /proc and triggers calls to get_pid_task
   */
  void trigger_get_pid_task(std::function<void(void)> periodic_cb);
};
