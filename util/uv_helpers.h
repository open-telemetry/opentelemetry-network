/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// This file contains a number of helper utilities to use when interacting with
// libuv.

#include <util/log.h>

#include <uv.h>

#include <sstream>

#include <functional>
#include <system_error>
#include <utility>

/**
 * This wrapper can be passed to logging functions and output streams with
 * error codes from libuv, so that error messages will be pretty printed.
 *
 * Error message resolution will take place lazily so that the cost or pretty
 * printing won't be paid upfront when a logging level is not going to be
 * printed.
 */
struct uv_error_t {
  inline uv_error_t(int error) : error_(error) {}

  int operator*() const { return error_; }

  bool operator!() const { return !error_; }
  explicit operator bool() const { return !!error_; }

  template <typename Out> friend Out &&operator<<(Out &&out, uv_error_t const &what)
  {
    char buffer[ERROR_BUFFER_SIZE];

    ::uv_err_name_r(*what, buffer, ERROR_BUFFER_SIZE);
    buffer[ERROR_BUFFER_SIZE - 1] = '\0';
    out << '<' << buffer << "> ";

    ::uv_strerror_r(*what, buffer, ERROR_BUFFER_SIZE);
    buffer[ERROR_BUFFER_SIZE - 1] = '\0';
    out << buffer;

    return std::forward<Out>(out);
  }

private:
  int error_;

  static constexpr std::size_t ERROR_BUFFER_SIZE = 128;
};

// Provide fmt support for uv_error_t using its ostream representation.
namespace fmt {
template <> struct formatter<uv_error_t> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(uv_error_t const &err, FormatContext &ctx) const
  {
    std::ostringstream os;
    os << err;
    return fmt::format_to(ctx.out(), "{}", os.str());
  }
};
} // namespace fmt

// Runs `fn` in a thread-safe manner within the `loop`'s thread and waits for
// its execution to finish before returning.
// NOTE: this is not optimized for speed - it's a blocking operation. Use only
// in test scenarios or application startup / shutdown where speed is not
// critical.
void sync_uv_run(::uv_loop_t &loop, std::function<void()> fn);

// Evaluates the libuv-error-returning expression. If an error occurs, will
// panic and log the error message.
#define CHECK_UV(...)                                                                                                          \
  {                                                                                                                            \
    const auto result = (__VA_ARGS__);                                                                                         \
    if (result != 0 /* success */) {                                                                                           \
      LOG::critical("libuv failure in {}:{} - {}", __FILE__, __LINE__, uv_strerror(result));                                   \
      std::exit(1);                                                                                                            \
    }                                                                                                                          \
  }

// Closes the loop and its associated handles. The caller is still responsible
// for freeing the handles' allocated data. This should be run after the loop
// has already stopped. Example:
//
//   uv_loop_t *loop = ....;
//   uv_run(loop, UV_RUN_DEFAULT);
//   close_uv_loop_cleanly(loop);
//
void close_uv_loop_cleanly(uv_loop_t *loop);

// Close the handle if it isn't already closed or in the process of being closed.
void close_uv_handle_cleanly(uv_handle_t *handle, void (*cb)(uv_handle_t *));

std::error_category const &libuv_category() noexcept;
