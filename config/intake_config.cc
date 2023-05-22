// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config/intake_config.h>

#include <channel/tcp_channel.h>
#include <util/environment_variables.h>
#include <util/log.h>

#include <util/utility.h>

#include <cstdlib>

namespace config {

FileDescriptor IntakeConfig::create_output_record_file() const
{
  FileDescriptor fd;

  LOG::debug("intake record file: `{}`", record_path_);
  if (!record_path_.empty()) {
    if (auto const error = fd.create(record_path_.c_str(), FileDescriptor::Access::write_only)) {
      LOG::error("failed to create intake record file at `{}`", record_path_);
    } else {
      LOG::debug("created intake record file at `{}`", record_path_);
    }
  }

  return fd;
}

std::unique_ptr<channel::NetworkChannel> IntakeConfig::make_channel(uv_loop_t &loop) const
{
  if (host_.empty()) {
    throw std::invalid_argument("missing intake host value");
  }

  if (port_.empty()) {
    throw std::invalid_argument("missing intake port value");
  }

  return std::make_unique<channel::TCPChannel>(loop, host_, port_);
}

IntakeConfig IntakeConfig::read_from_env()
{
  IntakeConfig intake{
      .host_ = std::string(try_get_env_var(INTAKE_HOST_VAR)),
      .port_ = std::string(try_get_env_var(INTAKE_PORT_VAR)),
      .record_output_path = std::string(try_get_env_var(INTAKE_RECORD_OUTPUT_PATH_VAR)),
      .encoder_ = try_get_env_value<IntakeEncoder>(INTAKE_INTAKE_ENCODER_VAR)};

  return intake;
}

IntakeConfig::ArgsHandler::ArgsHandler(cli::ArgsParser &parser)
    : host_(parser.add_arg<std::string>(
          "intake-host",
          "IP address or host name of the reducer to which telemetry is to be sent",
          INTAKE_HOST_VAR,
          "127.0.0.1")),
      port_(parser.add_arg<std::string>(
          "intake-port",
          "TCP port number on which the reducer is listening for collector connections",
          INTAKE_PORT_VAR,
          "8000")),
      encoder_(parser.add_arg(
          "intake-encoder",
          "Chooses the intake encoder to use"
          " - this relates to the sink used to dump collected telemetry to",
          INTAKE_INTAKE_ENCODER_VAR,
          IntakeEncoder::binary))
{}

IntakeConfig IntakeConfig::ArgsHandler::read_config()
{
  auto intake_config = config::IntakeConfig::read_from_env();

  intake_config.encoder(*encoder_);
  intake_config.host(*host_);
  intake_config.port(*port_);

  return intake_config;
}

} // namespace config
