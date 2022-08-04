/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>
#include <unordered_map>

#include "generated/collector.grpc.pb.h"
#include "generated/collector.pb.h"
#include "resync_queue_interface.h"

namespace collector {

// KubernetesRpcServer implements Collector::Service gRpc server.
//
// It recieves the client-side streaming gRpc from Kubernetes Reader, and
// extracts related information and forwards to the reducer.
class KubernetesRpcServer : public Collector::Service {
public:
  // Does not take ownership of |chanel_factory|
  explicit KubernetesRpcServer(ResyncChannelFactory *channel_factory, std::size_t collect_buffer_size);
  ~KubernetesRpcServer() override;

  ::grpc::Status Collect(::grpc::ServerContext *context, ::grpc::ServerReaderWriter<Response, Info> *reader_writer) override;

private:
  ResyncChannelFactory *channel_factory_; // not owned
  std::size_t collect_buffer_size_;
};
} // namespace collector
