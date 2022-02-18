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

#include <collector/aws/enumerator.h>
#include <collector/aws/ingest_connection.h>

#include <channel/callbacks.h>
#include <scheduling/interval_scheduler.h>
#include <scheduling/job.h>
#include <util/curl_engine.h>

#include <chrono>
#include <string>

namespace collector::aws {

struct AwsCollector : channel::Callbacks {
  AwsCollector(
      ::uv_loop_t &loop,
      std::string_view hostname,
      std::chrono::milliseconds aws_metadata_timeout,
      std::chrono::milliseconds heartbeat_interval,
      std::size_t buffer_size,
      config::IntakeConfig intake_config,
      std::chrono::milliseconds poll_interval);

  ~AwsCollector();

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

} // namespace collector::aws
