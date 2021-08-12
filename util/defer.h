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
