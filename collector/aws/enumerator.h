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

#include <common/collector_status.h>
#include <generated/flowmill/aws_collector/handles.h>
#include <scheduling/job.h>
#include <util/logger.h>

#include <aws/ec2/EC2Client.h>

#include <functional>
#include <vector>

namespace collector::aws {

struct NetworkInterfacesEnumerator {

  NetworkInterfacesEnumerator(logging::Logger &log, flowmill::aws_collector::Index &index, flowmill::ingest::Writer &writer);
  ~NetworkInterfacesEnumerator();

  scheduling::JobFollowUp enumerate();

  void free_handles();

private:
  void set_handles(std::vector<flowmill::aws_collector::handles::aws_network_interface> handles);

  void handle_ec2_error(CollectorStatus status, Aws::Client::AWSError<Aws::EC2::EC2Errors> const &error);

  Aws::EC2::EC2Client ec2_;
  flowmill::aws_collector::Index &index_;
  flowmill::ingest::Writer &writer_;
  logging::Logger &log_;
  std::vector<flowmill::aws_collector::handles::aws_network_interface> handles_;
};

} // namespace collector::aws
