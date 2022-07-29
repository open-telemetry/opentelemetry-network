/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <collector/cloud/enumerator.h>
#include <collector/cloud/ingest_connection.h>

#include <channel/callbacks.h>
#include <scheduling/interval_scheduler.h>
#include <scheduling/job.h>
#include <util/curl_engine.h>

#include <chrono>
#include <string>

namespace collector::cloud {

struct CloudCollector : channel::Callbacks {
  CloudCollector(
      ::uv_loop_t &loop,
      std::string_view hostname,
      std::chrono::milliseconds aws_metadata_timeout,
      std::chrono::milliseconds heartbeat_interval,
      std::size_t buffer_size,
      config::IntakeConfig intake_config,
      std::chrono::milliseconds poll_interval);

  ~CloudCollector();

  void run_loop();

  ::uv_loop_t &get_loop() { return loop_; }

private:
  scheduling::JobFollowUp callback();

  void on_error(int err);
  void on_connected();

  ::uv_loop_t &loop_;
  IngestConnection connection_;
  logging::Logger log_;
  NetworkInterfacesEnumerator enumerator_;
  scheduling::IntervalScheduler scheduler_;
  std::chrono::milliseconds const poll_interval_;
};

} // namespace collector::cloud
