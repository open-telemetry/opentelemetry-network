// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <fstream>

template <typename Index>
void IndexDumper::dump(std::string_view app, int shard, Index const &index, std::chrono::seconds timestamp)
{
  if (!cooldown_.count()) {
    return;
  }
  if (timestamp <= last_dump_ + cooldown_) {
    return;
  }
  last_dump_ = timestamp;

  auto const file_name =
      fmt::format("{}_{}-{}_{}.json", app, shard, iteration_++, integer_time<std::chrono::seconds>(timestamp));

  std::string file_path;
  if (!dump_dir_.empty()) {
    file_path = fmt::format("{}/{}", dump_dir_, file_name);
  } else {
    // Output in the working directory.
    file_path = file_name;
  }

  std::ofstream out{file_path};

  out << index;

  out.flush();
  out.close();
}
