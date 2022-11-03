/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/span_base.h>

namespace reducer::collector::cloud {

struct AwsNetworkInterfaceSpan : ebpf_net::ingest::AwsNetworkInterfaceSpanBase {
  AwsNetworkInterfaceSpan();
  ~AwsNetworkInterfaceSpan();

  void network_interface_info(
      ::ebpf_net::ingest::weak_refs::aws_network_interface span_ref, u64 timestamp, jsrv_ingest__network_interface_info *msg);

  void network_interface_info_deprecated(
      ::ebpf_net::ingest::weak_refs::aws_network_interface span_ref,
      u64 timestamp,
      jsrv_ingest__network_interface_info_deprecated *msg);
};

} // namespace reducer::collector::cloud
