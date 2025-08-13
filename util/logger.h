/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/writer.h>
#include <util/log.h>

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>

#include <utility>

namespace logging {

class Logger {
public:
  explicit Logger(::ebpf_net::ingest::Writer &writer) : writer_(writer) {}

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  template <typename Format, typename... Args> inline void info(Format &&format, Args &&... args)
  {
    LOG::info(log_message<spdlog::level::info>(std::forward<Format>(format), std::forward<Args>(args)...));
  }

  template <typename Format, typename... Args> inline void warn(Format &&format, Args &&... args)
  {
    LOG::warn(log_message<spdlog::level::warn>(std::forward<Format>(format), std::forward<Args>(args)...));
  }

  template <typename Format, typename... Args> inline void error(Format &&format, Args &&... args)
  {
    LOG::error(log_message<spdlog::level::err>(std::forward<Format>(format), std::forward<Args>(args)...));
  }

  template <typename Format, typename... Args> inline void critical(Format &&format, Args &&... args)
  {
    LOG::critical(log_message<spdlog::level::critical>(std::forward<Format>(format), std::forward<Args>(args)...));
  }

private:
  template <spdlog::level::level_enum Level, typename Format, typename... Args>
  inline std::string log_message(Format &&format, Args &&... args)
  {
    auto message = fmt::vformat(std::forward<Format>(format), fmt::make_format_args(args...));

    static_assert(static_cast<std::underlying_type_t<spdlog::level::level_enum>>(Level) == static_cast<u8>(Level));

    writer_.log_message(static_cast<u8>(Level), jb_blob{message});

    return message;
  }

  ::ebpf_net::ingest::Writer &writer_;
};

} // namespace logging
