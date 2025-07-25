// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config/intake_config.h>

#include <channel/tcp_channel.h>
#include <util/environment_variables.h>
#include <util/log.h>

#include <util/utility.h>

#include <cstdlib>

namespace config {

const IntakeConfig IntakeConfig::DEFAULT_CONFIG = IntakeConfig(
    "127.0.0.1",          // host
    "8000",               // port
    "",                   // record_output_path
    IntakeEncoder::binary // encoder
);

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

void IntakeConfig::read_from_env(IntakeConfig &config)
{
  if (std::string_view value = try_get_env_var(INTAKE_HOST_VAR); !value.empty()) {
    config.host_ = value;
  }

  if (std::string_view value = try_get_env_var(INTAKE_PORT_VAR); !value.empty()) {
    config.port_ = value;
  }

  if (std::string_view value = try_get_env_var(INTAKE_RECORD_OUTPUT_PATH_VAR); !value.empty()) {
    config.record_path_ = value;
  }

  if (std::string_view value = try_get_env_var(INTAKE_INTAKE_ENCODER_VAR); !value.empty()) {
    config.encoder_ = try_enum_from_string(value, IntakeEncoder::binary);
  }
}

IntakeConfig::ArgsHandler::ArgsHandler(cli::ArgsParser &parser)
    : host_(parser.add_arg<std::string>(
          "intake-host", "IP address or host name of the reducer to which telemetry is to be sent")),
      port_(parser.add_arg<std::string>(
          "intake-port", "TCP port number on which the reducer is listening for collector connections")),
      encoder_(parser.add_arg<IntakeEncoder>(
          "intake-encoder",
          "Chooses the intake encoder to use"
          " - this relates to the sink used to dump collected telemetry to"))
{}

void IntakeConfig::ArgsHandler::read_config(IntakeConfig &config)
{
  IntakeConfig::read_from_env(config);

  if (host_) {
    config.host(*host_);
  }

  if (port_) {
    config.port(*port_);
  }

  if (encoder_) {
    config.encoder(*encoder_);
  }
}

} // namespace config
