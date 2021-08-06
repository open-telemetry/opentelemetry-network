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

#include <config/intake_config.h>

#include <channel/tcp_channel.h>
#include <channel/tls_handler.h>
#include <util/environment_variables.h>
#include <util/log.h>

#include <util/utility.h>

#include <cstdlib>

namespace config {

FileDescriptor IntakeConfig::create_output_record_file() const {
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

std::unique_ptr<channel::NetworkChannel> IntakeConfig::make_channel(
  uv_loop_t &loop,
  std::string_view private_key,
  std::string_view certificate
) const {
  if (disable_tls_) {
    return std::make_unique<channel::TCPChannel>(loop, host_, port_, proxy_);
  }

  return std::make_unique<channel::TLSHandler>(
    loop, host_, port_,
    std::string{private_key},
    std::string{certificate},
    name_, proxy_
  );
}

IntakeConfig IntakeConfig::read_from_env() {
  IntakeConfig intake{
    .name_ = get_env_var(INTAKE_NAME_VAR),
    .host_ = get_env_var(INTAKE_HOST_VAR),
    .port_ = get_env_var(INTAKE_PORT_VAR),
    .proxy = config::HttpProxyConfig::read_from_env(),
    .record_output_path = std::string(try_get_env_var(INTAKE_RECORD_OUTPUT_PATH_VAR)),
    .disable_tls_ = try_get_env_value<bool>(INTAKE_DISABLE_TLS_VAR),
    .encoder_ = try_get_env_value<IntakeEncoder>(INTAKE_INTAKE_ENCODER_VAR),
    .auth_method_ = try_get_env_value<collector::AuthMethod>(INTAKE_AUTH_METHOD_VAR, collector::AuthMethod::authz)
  };

  return intake;
}

IntakeConfig IntakeConfig::read_from_env_and_intake(std::string_view intake_name) {
  LOG::trace_in(Utility::authz, "Got intake name from token: {}", intake_name);
  IntakeConfig intake{
    .name_ = std::string(try_get_env_var(INTAKE_NAME_VAR, intake_name)),
    .host_ = get_env_var(INTAKE_HOST_VAR),
    .port_ = get_env_var(INTAKE_PORT_VAR),
    .proxy = config::HttpProxyConfig::read_from_env(),
    .record_output_path = std::string(try_get_env_var(INTAKE_RECORD_OUTPUT_PATH_VAR)),
    .disable_tls_ = try_get_env_value<bool>(INTAKE_DISABLE_TLS_VAR),
    .encoder_ = try_get_env_value<IntakeEncoder>(INTAKE_INTAKE_ENCODER_VAR),
    .auth_method_ = try_get_env_value<collector::AuthMethod>(INTAKE_AUTH_METHOD_VAR, collector::AuthMethod::authz)
  };

  return intake;
}

IntakeConfig::ArgsHandler::ArgsHandler(cli::ArgsParser &parser):
  encoder_(
    parser.add_arg(
      "intake-encoder",
      "Chooses the intake encoder to use"
        " - this relates to the sink used to dump collected telemetry to",
      INTAKE_INTAKE_ENCODER_VAR,
      IntakeEncoder::binary
    )
  )
{}

IntakeConfig IntakeConfig::ArgsHandler::read_config(std::string_view intake_name) {
  auto intake_config = config::IntakeConfig::read_from_env_and_intake(intake_name);

  intake_config.encoder(*encoder_);

  return intake_config;
}

IntakeConfig IntakeConfig::ArgsHandler::read_config() {
  auto intake_config = config::IntakeConfig::read_from_env();

  intake_config.encoder(*encoder_);

  return intake_config;
}

} // namespace config {
