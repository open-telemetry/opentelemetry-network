// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "metrics_server_impl.h"

#include <util/args_parser.h>

#include <fstream>
#include <ostream>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
  cli::ArgsParser parser("otlp-to-prom");

  args::HelpFlag help(*parser, "help", "Display this help menu.", {'h', "help"});
  args::ValueFlag<u32> listen_port(*parser, "listen_port", "TCP port to listen for connections", {"listen-port"}, 4317);
  args::ValueFlag<std::string> out_file(*parser, "out_file", "File to write to", {"out-file"}, "metrics.txt");

  std::string bind = "0.0.0.0:" + std::to_string(listen_port.Get());
  MetricsServiceImpl::Sinks sinks;
  sinks.push_back(&std::cout);

  std::ofstream ofs(out_file.Get(), std::ios::out | std::ios::app);
  if (ofs) {
    sinks.push_back(&ofs);
  } else {
    std::cerr << "could not open " << out_file.Get() << " for appending." << std::endl;
    std::cerr << "only sending output to standard out." << std::endl;
  }

  MetricsServiceImpl service(std::move(sinks));

  grpc::ServerBuilder builder;
  builder.AddListeningPort(bind, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  auto the_server = builder.BuildAndStart();

  the_server->Wait();
  return 0;
}
