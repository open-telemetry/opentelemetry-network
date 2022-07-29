// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/cloud/collector.h>

#include <channel/component.h>
#include <collector/constants.h>
#include <common/cloud_platform.h>
#include <util/agent_id.h>
#include <util/args_parser.h>
#include <util/log.h>
#include <util/log_whitelist.h>
#include <util/signal_handler.h>
#include <util/system_ops.h>
#include <util/utility.h>

#include <aws/core/Aws.h>

#include <chrono>
#include <string>

#include <csignal>

/**
 * Cloud Collector Agent
 *
 * Requires AWS Access Key ID and Secret Access Key to be set up in the
 * environment:
 * https://docs.aws.amazon.com/general/latest/gr/aws-access-keys-best-practices.html
 *
 * In production, in an EC2 instance, this should work automagically.
 *
 * The easiest way to achieve that in a development environment is by setting up
 * environment variables `AWS_ACCESS_KEY_ID` and `AWS_SECRET_ACCESS_KEY`.
 */

int main(int argc, char *argv[])
{
  ::uv_loop_t loop;
  if (auto const error = ::uv_loop_init(&loop)) {
    throw std::runtime_error(::uv_strerror(error));
  }

  // args parsing

  cli::ArgsParser parser("Cloud Collector Agent");

  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});

  args::ValueFlag<std::chrono::milliseconds::rep> ec2_poll_interval_ms(
      *parser,
      "ec2_poll_interval_ms",
      "How often, in milliseconds, to enumerate interfaces in EC2.",
      {"ec2-poll-interval-ms"},
      std::chrono::milliseconds(1s).count());

  args::ValueFlag<u64> aws_metadata_timeout_ms(
      *parser, "milliseconds", "Milliseconds to wait for AWS instance metadata", {"aws-timeout"}, 1 * 1000);

  parser.new_handler<LogWhitelistHandler<channel::Component>>("channel");
  parser.new_handler<LogWhitelistHandler<CloudPlatform>>("cloud-platform");
  parser.new_handler<LogWhitelistHandler<Utility>>("utility");

  auto &intake_config_handler = parser.new_handler<config::IntakeConfig::ArgsHandler>();

  SignalManager &signal_manager = parser.new_handler<SignalManager>(loop, "cloud-collector");

  if (auto result = parser.process(argc, argv); !result.has_value()) {
    return result.error();
  }

  if (ec2_poll_interval_ms.Get() == 0) {
    LOG::error("--ec2-poll-interval-ms cannot be 0");
    return EXIT_FAILURE;
  }

  // resolve hostname
  std::string const hostname = get_host_name(MAX_HOSTNAME_LENGTH).recover([](auto &error) {
    LOG::error("Unable to retrieve host information from uname: {}", error);
    return "(unknown)";
  });

  auto curl_engine = CurlEngine::create(&loop);

  auto agent_id = gen_agent_id();

  auto intake_config = intake_config_handler.read_config();

  LOG::info("Cloud Collector version {} ({}) started on host {}", versions::release, release_mode_string, hostname);
  LOG::info("Cloud Collector agent ID is {}", agent_id);

  // aws sdk init

  Aws::InitAPI({});

  // main

  collector::cloud::CloudCollector collector{
      loop,
      hostname,
      std::chrono::milliseconds(aws_metadata_timeout_ms.Get()),
      HEARTBEAT_INTERVAL,
      WRITE_BUFFER_SIZE,
      std::move(intake_config),
      std::chrono::milliseconds(ec2_poll_interval_ms.Get())};

  signal_manager.handle_signals({SIGINT, SIGTERM} // TODO: close gracefully
  );

  collector.run_loop();

  // shutdown
  Aws::ShutdownAPI({});

  return EXIT_SUCCESS;
}
