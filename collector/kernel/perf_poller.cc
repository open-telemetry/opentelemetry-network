// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/perf_poller.h>

PerfPoller::PerfPoller(PerfContainer &container) : container_(container) {}

PerfPoller::~PerfPoller() {}

void PerfPoller::start(u64 interval_useconds, u64 n_intervals)
{
  while (n_intervals) {
    poll();
    usleep(interval_useconds);
    n_intervals--;
  }
}
