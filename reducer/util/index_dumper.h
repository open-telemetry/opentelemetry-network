/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

class IndexDumper {
public:
  template <typename Index> void dump(std::string_view app, int shard, Index const &index, std::chrono::seconds timestamp);

  // Sets the output directory.
  // If the output directory is not set, then the process' working directory is used.
  // This function is not thread-safe and should be called by `main` before any threads are created.
  static void set_dump_dir(std::string_view dir);

  // This function is not thread-safe and should be called by `main` before any threads are created.
  // A value of 0 (default) disables index dumping.
  static void set_cooldown(std::chrono::seconds cooldown);

private:
  std::chrono::seconds last_dump_ = {};
  std::size_t iteration_ = 0;

  static std::string dump_dir_;
  static std::chrono::seconds cooldown_;
};

#include <reducer/util/index_dumper.inl>
