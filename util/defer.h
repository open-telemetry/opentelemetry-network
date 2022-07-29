/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/preprocessor.h>

#include <functional>

// Defer is a convenience class for executing a function upon leaving scope
// (similar to what Go's `defer` statement does). Saves the effort of creating
// a custom RAII-based class for simple tasks.
//
// Example usage:
//
// void do_something() {
//   void* data = malloc(1234);
//   Defer free_data([&] { free(data); });
//   ...
// }
class Defer {
public:
  explicit Defer(std::function<void()> cb) : cb_(std::move(cb)) {}
  ~Defer() { cb_(); }

  // Move-only.
  Defer(const Defer &) = delete;
  Defer(Defer &&) = default;

private:
  std::function<void()> cb_;
};

// DEFER is a convenience macro to declare a Defer class and associated callback function.
//
// Example usage:
//
// void do_something() {
//   void* data = malloc(1234);
//   DEFER([&] { free(data); });
//   ...
// }
#define DEFER(cb) Defer PREPROC_CONCAT(defer_, __LINE__)(cb)
