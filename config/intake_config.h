/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/network_channel.h>
#include <common/intake_encoder.h>
#include <util/args_parser.h>
#include <util/file_ops.h>

#include <generated/ebpf_net/ingest/encoder.h>

#include <spdlog/fmt/ostr.h>

#include <uv.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace config {

class IntakeConfig {
  // environment variable names used by `read_from_env()`
  static constexpr auto INTAKE_HOST_VAR = "EBPF_NET_INTAKE_HOST";
  static constexpr auto INTAKE_PORT_VAR = "EBPF_NET_INTAKE_PORT";
  static constexpr auto INTAKE_INTAKE_ENCODER_VAR = "EBPF_NET_INTAKE_ENCODER";
  static constexpr auto INTAKE_RECORD_OUTPUT_PATH_VAR = "EBPF_NET_RECORD_INTAKE_OUTPUT_PATH";

public:
  static const IntakeConfig DEFAULT_CONFIG;

  IntakeConfig() {}

  /**
   * Constructs an intake config object.
   *
   * host: the host to connect to for intake
   * port: the port to connect to for intake
   * secondary_output: when given, path to a file into which to record all traffic sent upstream
   */
  IntakeConfig(
      std::string host,
      std::string port,
      std::string record_output_path = {},
      IntakeEncoder encoder = IntakeEncoder::binary)
      : host_(std::move(host)),
        port_(std::move(port)),
        record_path_(std::move(record_output_path)),
        encoder_(encoder)
  {}

  std::string const &host() const { return host_; }
  std::string const &port() const { return port_; }

  void host(std::string const &host) { host_ = host; }
  void port(std::string const &port) { port_ = port; }

  /**
   * If a secondary output has been set, opens or creates the output file and
   * returns its file descriptor.
   *
   * Otherwise, returns an invalid file descriptor.
   */
  FileDescriptor create_output_record_file() const;

  void encoder(IntakeEncoder encoder) { encoder_ = encoder; }
  IntakeEncoder encoder() const { return encoder_; }

  virtual bool allow_compression() const { return encoder_ == IntakeEncoder::binary; }

  virtual std::unique_ptr<channel::NetworkChannel> make_channel(uv_loop_t &loop) const;

  std::unique_ptr<::ebpf_net::ingest::Encoder> make_encoder() const
  {
    switch (encoder_) {
    default:
      // Use default encoder.
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
  static void read_from_env(IntakeConfig &config);

  template <typename Out> friend Out &&operator<<(Out &&out, IntakeConfig const &config)
  {
    out << config.host_ << ':' << config.port_ << " (" << config.encoder_ << ')';

    return std::forward<Out>(out);
  }

  struct ArgsHandler;

private:
  std::string host_;
  std::string port_;
  std::string record_path_;
  IntakeEncoder encoder_ = IntakeEncoder::binary;
};

struct IntakeConfig::ArgsHandler : cli::ArgsParser::Handler {
  ArgsHandler(cli::ArgsParser &parser);

  void read_config(IntakeConfig &config);

private:
  cli::ArgsParser::ArgProxy<std::string> host_;
  cli::ArgsParser::ArgProxy<std::string> port_;
  cli::ArgsParser::ArgProxy<IntakeEncoder> encoder_;
};

} // namespace config

namespace fmt {
template <> struct formatter<config::IntakeConfig> : fmt::ostream_formatter {};
} // namespace fmt
