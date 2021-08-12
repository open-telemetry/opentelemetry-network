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

#include <collector/aws/collector.h>

#include <channel/component.h>
#include <collector/component.h>
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
 * AWS Collector Agent
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

  // read config from environment

  auto const agent_key = AuthzFetcher::read_agent_key()
                             .on_error([](auto &error) {
                               LOG::critical("Authentication key error: {}", error);
                               exit(-1);
                             })
                             .value();

  // args parsing

  cli::ArgsParser parser("Flowmill AWS collector agent");

  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});

  args::ValueFlag<std::chrono::milliseconds::rep> ec2_poll_interval_ms(
      *parser,
      "ec2_poll_interval_ms",
      "How often, in milliseconds, to enumerate interfaces in EC2.",
      {"ec2-poll-interval-ms"},
      std::chrono::milliseconds(1s).count());

  auto &authz_server = AuthzFetcher::register_args_parser(parser);

  args::ValueFlag<u64> aws_metadata_timeout_ms(
      *parser, "milliseconds", "Milliseconds to wait for AWS instance metadata", {"aws-timeout"}, 1 * 1000);

  parser.new_handler<LogWhitelistHandler<channel::Component>>("channel");
  parser.new_handler<LogWhitelistHandler<collector::Component>>("component");
  parser.new_handler<LogWhitelistHandler<CloudPlatform>>("cloud-platform");
  parser.new_handler<LogWhitelistHandler<Utility>>("utility");

  auto &intake_config_handler = parser.new_handler<config::IntakeConfig::ArgsHandler>();

  SignalManager &signal_manager =
      parser.new_handler<SignalManager>(loop, "aws-collector").add_auth(agent_key.key_id, agent_key.secret);

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

  // Fetch initial authz token
  auto maybe_proxy_config = config::HttpProxyConfig::read_from_env();
  auto proxy_config = maybe_proxy_config.has_value() ? &maybe_proxy_config.value() : nullptr;
  AuthzFetcher authz_fetcher{*curl_engine, *authz_server, agent_key, agent_id, proxy_config};

  auto intake_config = intake_config_handler.read_config(authz_fetcher.token()->intake());

  LOG::info("AWS Collector version {} ({}) started on host {}", versions::release, release_mode_string, hostname);
  LOG::info("AWS Collector agent ID is {}", agent_id);

  // aws sdk init

  Aws::InitAPI({});

  // main

  collector::aws::AwsCollector collector{
      loop,
      hostname,
      authz_fetcher,
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
