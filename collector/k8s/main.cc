// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/component.h>
#include <channel/reconnecting_channel.h>
#include <collector/constants.h>
#include <collector/k8s/kubernetes_rpc_server.h>
#include <collector/k8s/resync_processor.h>
#include <collector/k8s/resync_queue.h>
#include <common/cloud_platform.h>
#include <config/config_file.h>
#include <util/agent_id.h>
#include <util/args_parser.h>
#include <util/log.h>
#include <util/log_whitelist.h>
#include <util/signal_handler.h>
#include <util/system_ops.h>
#include <util/utility.h>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <yaml-cpp/yaml.h>

#include <csignal>
#include <cstdlib>
#include <thread>

using ::grpc::Server;
using ::grpc::ServerBuilder;

void run_uv_loop(channel::ReconnectingChannel *channel, uv_loop_t *loop)
{
  for (;;) {
    channel->start_connect();
    uv_run(loop, UV_RUN_DEFAULT);
  }
}

int main(int argc, char *argv[])
{
  ::uv_loop_t loop;
  if (auto const error = ::uv_loop_init(&loop)) {
    throw std::runtime_error(::uv_strerror(error));
  }

  // args parsing

  cli::ArgsParser parser("Kubernetes Collector relay service");

  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});

  args::ValueFlag<std::string> server_address(
      *parser,
      "server_address",
      "The address, in HOST:PORT format, of this relay service",
      {"server-address"},
      "localhost:8712");

  args::ValueFlag<std::string> conf_file(*parser, "config_file", "The location of the custom config file", {"config-file"}, "");

  args::ValueFlag<u64> aws_metadata_timeout_ms(
      *parser, "milliseconds", "Milliseconds to wait for AWS instance metadata", {"aws-timeout"}, 1 * 1000);
  args::ValueFlag<u16> heartbeat_interval_sec(
      *parser,
      "heartbeat_interval_sec",
      "Seconds between heartbeat messages sent to the pipeline server.",
      {"heartbeat-interval-sec"},
      std::chrono::duration_cast<std::chrono::seconds>(HEARTBEAT_INTERVAL).count());

  parser.new_handler<LogWhitelistHandler<channel::Component>>("channel");
  parser.new_handler<LogWhitelistHandler<CloudPlatform>>("cloud-platform");
  parser.new_handler<LogWhitelistHandler<Utility>>("utility");

  auto &intake_config_handler = parser.new_handler<config::IntakeConfig::ArgsHandler>();

  SignalManager &signal_manager = parser.new_handler<SignalManager>(loop, "k8s-collector");

  if (auto result = parser.process(argc, argv); !result.has_value()) {
    return result.error();
  }

  auto agent_id = gen_agent_id();

  // resolve hostname
  std::string const hostname = get_host_name(MAX_HOSTNAME_LENGTH).recover([](auto &error) {
    LOG::error("Unable to retrieve host information from uname: {}", error);
    return "(unknown)";
  });
  LOG::info("Kubernetes Collector version {} ({}) started on host {}", versions::release, release_mode_string, hostname);
  LOG::info("Kubernetes Collector agent ID is {}", agent_id);

  config::ConfigFile configuration_data(config::ConfigFile::YamlFormat(), conf_file.Get());

  signal_manager.handle_signals({SIGINT, SIGTERM} // TODO: close gracefully
  );

  auto curl_engine = CurlEngine::create(&loop);

  auto intake_config = intake_config_handler.read_config();

  channel::ReconnectingChannel channel(std::move(intake_config), loop, WRITE_BUFFER_SIZE);
  collector::ResyncQueue queue;

  collector::ResyncProcessor processor{
      loop,
      &queue,
      channel,
      configuration_data,
      hostname,
      std::chrono::milliseconds(aws_metadata_timeout_ms.Get()),
      std::chrono::seconds(heartbeat_interval_sec.Get()),
      WRITE_BUFFER_SIZE};
  channel.register_pipeline_observer(&processor);
  collector::KubernetesRpcServer service(&queue, WRITE_BUFFER_SIZE);

  std::thread uv_thread(run_uv_loop, &channel, &loop);

  ServerBuilder builder;
  builder.AddListeningPort(server_address.Get(), grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  server->Wait();

  uv_thread.join();

  // Above function should never return.
  return EXIT_FAILURE;
}
