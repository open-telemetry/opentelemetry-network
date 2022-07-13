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

#include <platform/types.h>
#include <util/environment_variables.h>
#include <util/log.h>

// clang-format off
// The order of following includes is important for the spdlog library.
// Disable clang-format so that it won't reorder them.
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
// clang-format on
#include <chrono>
#include <memory>
#include <cstdlib>

namespace {
constexpr unsigned int MAX_FILE_SIZE = 1024 * 1024;
constexpr unsigned int MAX_NUM_FILES = 5;
constexpr auto FLUSH_INTERVAL = 5s;

constexpr std::string_view LOG_FILE_VAR = "EBPF_NET_LOG_FILE_PATH";
constexpr std::string_view LOG_FILE_DEFAULT = "/var/log/flowmill.log";
} // namespace

// TODO: move the rate limit mechanism into a spdlog sink (inside its lock),
//       just before the line is written out. This way, there is no need
//       to use atomic operation.
std::atomic<int64_t> LOG::rate_limit_budget(0);
bool LOG::rate_limit_enabled = false;

void LOG::init(bool console, std::string const *filename)
{
  std::vector<spdlog::sink_ptr> sinks;

  if (filename) {
    sinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(*filename, MAX_FILE_SIZE, MAX_NUM_FILES));
  }

  if (console) {
    if (auto const fd = fileno(stdout); fd != -1 && isatty(fd)) {
      // colored output if stdout is a tty
      sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    } else {
      sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }
  }

  if (sinks.empty()) {
    sinks.emplace_back(std::make_shared<spdlog::sinks::null_sink_mt>());
  }

  // thread pool values borrowed from spdlog's defaults
  spdlog::init_thread_pool(8192, 1);
  auto logger = std::make_shared<spdlog::async_logger>(
      "main_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

  spdlog::set_default_logger(std::move(logger));

  spdlog::set_pattern("%Y-%m-%d %T.%f%z %^%l%$ [p:%P t:%t] %v");

  spdlog::flush_on(spdlog::level::err);

  spdlog::flush_every(FLUSH_INTERVAL);
}

std::string_view LOG::log_file_path()
{
  return try_get_env_var(LOG_FILE_VAR.data(), LOG_FILE_DEFAULT);
}

void LOG::enable_rate_limit(int64_t initial_budget)
{
  assert(initial_budget > 0);

  LOG::rate_limit_budget = initial_budget;
  LOG::rate_limit_enabled = true;
}

bool LOG::rate_limited()
{
  if (!LOG::rate_limit_enabled) {
    return false;
  }

  if (LOG::rate_limit_budget.fetch_sub(1) <= 0) {
    return true;
  }

  return false;
}

void LOG::refill_rate_limit_budget(int64_t amount)
{
  if (!LOG::rate_limit_enabled) {
    return;
  }

  assert(amount > 0);

  // Try a few times to update the budget atomically
  for (int i = 0; i < 2; i++) {
    int64_t current = LOG::rate_limit_budget.load();
    if (LOG::rate_limit_budget.compare_exchange_strong(current, std::max(current, amount))) {
      return;
    }
  }

  // Gives up, just update it to |amount|.
  // We might leak |amount| messages budget.
  LOG::rate_limit_budget.store(amount);
}
