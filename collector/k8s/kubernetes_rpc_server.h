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
#include <string>
#include <unordered_map>

#include "generated/collector.grpc.pb.h"
#include "generated/collector.pb.h"
#include "resync_queue_interface.h"

namespace collector {

// KubernetesRpcServer implements Collector::Service gRpc server.
//
// It recieves the client-side streaming gRpc from Kubernetes Reader, and
// extracts related information and forwards to Flowmill pipeline server.
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
