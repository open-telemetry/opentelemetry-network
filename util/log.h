/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <spdlog/cfg/env.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <config.h>
#include <string>
#include <string_view>

#include <common/client_type.h>
#include <util/debug.h>
#include <util/log_modifiers.h>
#include <util/log_whitelist.h>
#include <util/fmt_extensions.h>

class LOG {
public:
  /**
   * Initialize loggers.
   *
   * If `console` is true, intializes console log sink.
   *
   * If `filename` is not null, initializes file log sink.
   */
  static void init(bool console, std::string const *filename = nullptr);

  /**
   * Resolve log file path from environment variable.
   */
  static std::string_view log_file_path();

  // Enables global rate limit
  // It is expected that this function is called once during global
  // initialization (e.g., in main()).
  //
  // |initial_budget|: number of messages allowed initiallly
  static void enable_rate_limit(int64_t initial_budget);

  // Bumps up the max number of messages allowed by |amount|.
  static void refill_rate_limit_budget(int64_t amount);

  template <typename Format, typename... Args> static void inline trace(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();

    if (rate_limited()) {
      return;
    }
    spdlog::trace(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);

  }

  /**
   * Calls trace() if the given filter is in the log whitelist defined by
   * `Whitelist`. See `util/log_whitelist.h` for details.
   *
   * Example:
   *
   * LOG::trace_in(client_type_, "my log string");
   */
  template <typename Whitelist, typename Format, typename... Args, typename = std::enable_if_t<std::is_enum_v<Whitelist>>>
  static void inline trace_in(Whitelist filter, Format &&format, Args &&... args)
  {
    if (!is_log_whitelisted(filter)) {
      return;
    }
    trace(std::forward<Format>(format), std::forward<Args>(args)...);
  }

  /**
   * Calls trace() if all given filters are in the log whitelist defined by
   * `Whitelist`. See `util/log_whitelist.h` for details.
   *
   * Example:
   *
   * LOG::trace_in(client_type_, NodeResolutionType::AWS, "my log string");
   */
  template <typename Format, typename... Whitelist, typename... Args>
  static void inline trace_in(std::tuple<Whitelist...> const &filter, Format &&format, Args &&... args)
  {
    if (!is_log_whitelisted(filter)) {
      return;
    }
    trace(std::forward<Format>(format), std::forward<Args>(args)...);
  }

  template <typename Format, typename... Args> static void inline debug(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();
    if (rate_limited()) {
      return;
    }
    spdlog::debug(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);
  }

  /**
   * Calls debug() if the given filter is in the log whitelist defined by
   * `Whitelist`. See `util/log_whitelist.h` for details.
   *
   * Example:
   *
   * LOG::debug_in(client_type_, "my log string");
   *
   * LOG::debug_in(client_type_, NodeResolutionType::AWS, "my log string");
   */
  template <typename Whitelist, typename Format, typename... Args, typename = std::enable_if_t<std::is_enum_v<Whitelist>>>
  static void inline debug_in(Whitelist filter, Format &&format, Args &&... args)
  {
    if (!is_log_whitelisted(filter)) {
      return;
    }
    debug(std::forward<Format>(format), std::forward<Args>(args)...);
  }

  /**
   * Calls debug() if all given filters are in the log whitelist defined by
   * `Whitelist`. See `util/log_whitelist.h` for details.
   *
   * Example:
   *
   * LOG::debug_in(client_type_, NodeResolutionType::AWS, "my log string");
   */
  template <typename Format, typename... Whitelist, typename... Args>
  static void inline debug_in(std::tuple<Whitelist...> const &filter, Format &&format, Args &&... args)
  {
    if (!is_log_whitelisted(filter)) {
      return;
    }
    debug(std::forward<Format>(format), std::forward<Args>(args)...);
  }

  template <typename Format, typename... Args> static void inline info(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();
    if (rate_limited()) {
      return;
    }
    spdlog::info(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);
  }

  template <typename Format, typename... Args> static void inline warn(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();
    if (rate_limited()) {
      return;
    }
    spdlog::warn(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);
  }

  template <typename Format, typename... Args> static void inline error(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();
    if (rate_limited()) {
      return;
    }
    spdlog::error(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);
  }

  template <typename Format, typename... Args> static void inline critical(Format &&format, Args &&... args)
  {
    logger::check_logging_overhead<Args &&...>();
    if (rate_limited()) {
      return;
    }
    spdlog::critical(fmt::runtime(std::forward<Format>(format)), std::forward<Args>(args)...);
  }

private:
  // Returns true if rate limit is hit, and we should not log for now.
  static bool rate_limited();

  static bool rate_limit_enabled;
  static std::atomic<int64_t> rate_limit_budget;
};
