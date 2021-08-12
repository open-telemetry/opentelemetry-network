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

// This file contains a number of helper utilities to use when interacting with
// libuv.

#include <util/log.h>

#include <uv.h>

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

std::error_category const &libuv_category() noexcept;
