/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/collector_status.h>
#include <generated/ebpf_net/cloud_collector/handles.h>
#include <scheduling/job.h>
#include <util/logger.h>

#include <aws/ec2/EC2Client.h>

#include <functional>
#include <vector>

namespace collector::cloud {

struct NetworkInterfacesEnumerator {

  NetworkInterfacesEnumerator(logging::Logger &log, ebpf_net::cloud_collector::Index &index, ebpf_net::ingest::Writer &writer);
  ~NetworkInterfacesEnumerator();

  scheduling::JobFollowUp enumerate();

  void free_handles();

private:
  void set_handles(std::vector<ebpf_net::cloud_collector::handles::aws_network_interface> handles);

  void handle_ec2_error(CollectorStatus status, Aws::Client::AWSError<Aws::EC2::EC2Errors> const &error);

  Aws::EC2::EC2Client ec2_;
  ebpf_net::cloud_collector::Index &index_;
  ebpf_net::ingest::Writer &writer_;
  logging::Logger &log_;
  std::vector<ebpf_net::cloud_collector::handles::aws_network_interface> handles_;
};

} // namespace collector::cloud
