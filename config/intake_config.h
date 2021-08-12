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

#include <channel/network_channel.h>
#include <collector/auth_method.h>
#include <common/intake_encoder.h>
#include <config/http_proxy_config.h>
#include <util/args_parser.h>
#include <util/authz_token.h>
#include <util/file_ops.h>

#include <generated/flowmill/ingest/encoder.h>
#include <generated/flowmill/ingest/otlp_log_encoder.h>

#include <uv.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace config {

class IntakeConfig {
  // environment variable names used by `read_from_env()`
  static constexpr auto INTAKE_NAME_VAR = "FLOWMILL_INTAKE_NAME";
  static constexpr auto INTAKE_HOST_VAR = "FLOWMILL_INTAKE_HOST";
  static constexpr auto INTAKE_PORT_VAR = "FLOWMILL_INTAKE_PORT";
  static constexpr auto INTAKE_DISABLE_TLS_VAR = "FLOWMILL_INTAKE_DISABLE_TLS";
  static constexpr auto INTAKE_INTAKE_ENCODER_VAR = "FLOWMILL_INTAKE_ENCODER";
  static constexpr auto INTAKE_RECORD_OUTPUT_PATH_VAR = "FLOWMILL_RECORD_INTAKE_OUTPUT_PATH";
  static constexpr auto INTAKE_AUTH_METHOD_VAR = "FLOWMILL_INTAKE_AUTH_METHOD";

public:
  IntakeConfig() {}

  /**
   * Constructs an intake config object.
   *
   * name: the server name to use with SNI (https://en.wikipedia.org/wiki/Server_Name_Indication)
   * host: the host to connect to for intake
   * port: the port to connect to for intake
   * secondary_output: when given, path to a file into which to record all traffic sent upstream
   */
  IntakeConfig(
      std::string name,
      std::string host,
      std::string port,
      std::optional<config::HttpProxyConfig> proxy = {},
      std::string record_output_path = {},
      bool disable_tls = false,
      IntakeEncoder encoder = IntakeEncoder::binary,
      collector::AuthMethod auth_method = collector::AuthMethod::authz)
      : name_(std::move(name)),
        host_(std::move(host)),
        port_(std::move(port)),
        proxy_(std::move(proxy)),
        record_path_(std::move(record_output_path)),
        disable_tls_(disable_tls),
        encoder_(encoder),
        auth_method_(auth_method)
  {}

  std::string const &name() const { return name_; }
  std::string const &host() const { return host_; }
  std::string const &port() const { return port_; }

  config::HttpProxyConfig const *proxy() const { return proxy_ ? &*proxy_ : nullptr; }

  /**
   * If a secondary output has been set, opens or creates the output file and
   * returns its file descriptor.
   *
   * Otherwise, returns an invalid file descriptor.
   */
  FileDescriptor create_output_record_file() const;

  void disable_tls(bool disable) { disable_tls_ = disable; }
  bool disable_tls() const { return disable_tls_; }

  void encoder(IntakeEncoder encoder) { encoder_ = encoder; }
  IntakeEncoder encoder() const { return encoder_; }

  collector::AuthMethod auth_method() const { return auth_method_; }

  bool allow_compression() const { return encoder_ == IntakeEncoder::binary; }

  std::unique_ptr<channel::NetworkChannel>
  make_channel(uv_loop_t &loop, std::string_view private_key = {}, std::string_view certificate = {}) const;

  std::unique_ptr<::flowmill::ingest::Encoder> make_encoder() const
  {
    switch (encoder_) {
    case IntakeEncoder::otlp_log:
      return std::make_unique<::flowmill::ingest::OtlpLogEncoder>(host_, port_);

    default:
      return nullptr;
    }
  }

  /**
   * Reads intake configuration from existing environment variables.
   *
   * NOTE: this function reads environment variables so it's advisable to call it
   * before any thread is created, given that reading/writing to environment
   * variables is not thread safe and we can't control 3rd party libraries.
   */
  static IntakeConfig read_from_env();

  /**
   * Reads intake configuration from existing environment variables, an authz token, and a proxy config
   *
   * The intake_name is read from the authz token
   *
   * NOTE: this function reads environment variables so it's advisable to call it
   * before any thread is created, given that reading/writing to environment
   * variables is not thread safe and we can't control 3rd party libraries.
   */
  static IntakeConfig read_from_env_and_intake(std::string_view intake_name);

  template <typename Out> friend Out &&operator<<(Out &&out, IntakeConfig const &config)
  {
    constexpr char const *tls_label[2] = {"tls", "tcp"};

    out << config.name_ << " @ " << config.host_ << ':' << config.port_ << " (" << tls_label[config.disable_tls_] << ' '
        << config.encoder_;
    if (config.proxy_) {
      out << " / proxy @ " << config.proxy_->host() << ':' << config.proxy_->port();
    }
    out << ')';

    return std::forward<Out>(out);
  }

  struct ArgsHandler;

private:
  std::string name_;
  std::string host_;
  std::string port_;
  std::optional<config::HttpProxyConfig> proxy_;
  std::string record_path_;
  bool disable_tls_ = false;
  IntakeEncoder encoder_ = IntakeEncoder::binary;
  collector::AuthMethod auth_method_ = collector::AuthMethod::authz;
};

struct IntakeConfig::ArgsHandler : cli::ArgsParser::Handler {
  ArgsHandler(cli::ArgsParser &parser);

  IntakeConfig read_config(std::string_view intake_name);
  IntakeConfig read_config();

private:
  cli::ArgsParser::ArgProxy<IntakeEncoder> encoder_;
};

} // namespace config
