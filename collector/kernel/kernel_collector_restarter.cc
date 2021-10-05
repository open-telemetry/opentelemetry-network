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

#include <collector/kernel/kernel_collector.h>
#include <collector/kernel/kernel_collector_restarter.h>

KernelCollectorRestarter::KernelCollectorRestarter(KernelCollector &collector) : collector_(collector)
{
  reset();
}

void KernelCollectorRestarter::check_restart()
{
  if (startup_completed_ && restart_requested_) {
    if (restart_in_progress_) {
      LOG::debug("KernelCollector restart already in progress");
    } else {
      restart_in_progress_ = true;
      LOG::debug("restarting KernelCollector");
      collector_.restart();
    }
  }
}

void KernelCollectorRestarter::startup_completed()
{
  LOG::debug("KernelCollectorRestarter: startup completed");
  startup_completed_ = true;
  check_restart();
}

void KernelCollectorRestarter::request_restart()
{
  LOG::debug("KernelCollectorRestarter: restart requested");
  restart_requested_ = true;
  check_restart();
}

void KernelCollectorRestarter::reset()
{
  startup_completed_ = false;
  restart_requested_ = false;
  restart_in_progress_ = false;
}
