/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/collector_status.h>
#include <generated/flowmill/cloud_collector/handles.h>
#include <scheduling/job.h>
#include <util/logger.h>

#include <aws/ec2/EC2Client.h>

#include <functional>
#include <vector>

namespace collector::cloud {

struct NetworkInterfacesEnumerator {

  NetworkInterfacesEnumerator(logging::Logger &log, flowmill::cloud_collector::Index &index, flowmill::ingest::Writer &writer);
  ~NetworkInterfacesEnumerator();

  scheduling::JobFollowUp enumerate();

  void free_handles();

private:
  void set_handles(std::vector<flowmill::cloud_collector::handles::aws_network_interface> handles);

  void handle_ec2_error(CollectorStatus status, Aws::Client::AWSError<Aws::EC2::EC2Errors> const &error);

  Aws::EC2::EC2Client ec2_;
  flowmill::cloud_collector::Index &index_;
  flowmill::ingest::Writer &writer_;
  logging::Logger &log_;
  std::vector<flowmill::cloud_collector::handles::aws_network_interface> handles_;
};

} // namespace collector::cloud
