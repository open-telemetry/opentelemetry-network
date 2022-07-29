// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
