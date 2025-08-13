/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// This file contains helpers for performing error handling in code.

#include <absl/strings/str_cat.h>
#include <spdlog/fmt/fmt.h>

#include <iostream>
#include <sstream>
#include <string>

// This macro is like a more expressive version of the built-in assert function.
// It will evaluate the provided expression and exit if it's false, otherwise
// it does nothing. This will still evaluate, even if not compiled in debug mode
// (unlike `assert(...)`. Among the details included in the error message are:
//   - The time of failure (via spdlog)
//   - The file + line of the ASSUME statement
//   - A stacktrace including demangled symbols
//
// Furthermore, custom messages can be added useing the `else_log` function,
// which uses spdlog-compatible format strings.
// Examples:
//
//    bool success = should_always_return_true();
//    ASSUME(success);
//
//    int size = compute_size_of_thing(thing);
//    ASSUME(size == 0).else_log("Size of thing is {} instead of zero!", size);
//
// None of the expressions in the custom message are evaluated if the
// conditional evaluates to true.
//
#define ASSUME(...)                                                                                                            \
  (__VA_ARGS__) || ::internal::Panicker(::absl::StrCat("Assumption `", #__VA_ARGS__, "` failed at ", __FILE__, ":", __LINE__))

// DEBUG_ASSUME - version of the ASSUME macro enabled only on debug builds.
// To be used when the evaluation of the provided expression is too costly for
// release builds.
//
#ifdef NDEBUG
#define DEBUG_ASSUME(...) (::internal::NopPanicker{})
#else
#define DEBUG_ASSUME ASSUME
#endif // NDEBUG

namespace internal {

// Internal class that prints a message. Exits and dumps everything in the
// destructor.
struct Panicker {
  // The first line of the error message (the remaining ones being the stack
  // trace).
  explicit Panicker(std::string preamble) : preamble_(std::move(preamble)) {}

  // Program exits here.
  [[noreturn]] ~Panicker();

  // Custom message formatter. Uses spdlog-like formatting.
  template <typename... Args> Panicker &else_log(fmt::format_string<Args...> format, Args &&... args)
  {
    fmt::memory_buffer out;
#if FMT_VERSION >= 60000
    fmt::format_to(std::back_inserter(out), format, std::forward<Args>(args)...);
#else
    fmt::format_to(out, format, std::forward<Args>(args)...);
#endif
    custom_message_ = fmt::to_string(out);
    return *this;
  }

  // Needed so that the `(expression) || Panicker(...)` statement compiles.
  operator bool() const { return true; }

private:
  const std::string preamble_;
  std::string custom_message_;
};

#ifdef NDEBUG
// Dummy panicker used by DEBUG_ASSUME on non-debug builds.
struct NopPanicker {
  template <typename... Args> NopPanicker &else_log(Args &&... args) { return *this; }
  operator bool() const { return true; }
};
#endif // NDEBUG

} // namespace internal
