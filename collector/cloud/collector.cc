// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/cloud/collector.h>

#include <scheduling/interval_scheduler.h>

#include <util/jitter.h>
#include <util/log.h>
#include <util/log_formatters.h>

#include <functional>
#include <stdexcept>
#include <thread>

namespace collector::cloud {

namespace {

constexpr auto RECONNECT_DELAY = 5s;
// explicitly using milliseconds for a finer grained jitter
constexpr std::chrono::milliseconds RECONNECT_JITTER = 1s;

} // namespace

CloudCollector::CloudCollector(
    ::uv_loop_t &loop,
    std::string_view hostname,
    std::chrono::milliseconds aws_metadata_timeout,
    std::chrono::milliseconds heartbeat_interval,
    std::size_t buffer_size,
    config::IntakeConfig intake_config,
    std::chrono::milliseconds poll_interval)
    : loop_(loop),
      connection_(
          hostname,
          loop_,
          aws_metadata_timeout,
          heartbeat_interval,
          std::move(intake_config),
          buffer_size,
          *this,
          std::bind(&CloudCollector::on_connected, this)),
      log_(connection_.writer()),
      enumerator_(log_, connection_.index(), connection_.writer()),
      scheduler_(loop_, std::bind(&CloudCollector::callback, this)),
      poll_interval_(poll_interval)
{}

CloudCollector::~CloudCollector()
{
  ::uv_loop_close(&loop_);
}

void CloudCollector::run_loop()
{
  connection_.connect();

  while (::uv_run(&loop_, UV_RUN_DEFAULT))
    ;

  scheduler_.stop();
}

scheduling::JobFollowUp CloudCollector::callback()
{
  auto result = enumerator_.enumerate();
  connection_.flush();
  return result;
}

void CloudCollector::on_error(int err)
{
  scheduler_.stop();

  enumerator_.free_handles();

  std::this_thread::sleep_for(add_jitter(RECONNECT_DELAY, -RECONNECT_JITTER, RECONNECT_JITTER));
}

void CloudCollector::on_connected()
{
  scheduler_.start(poll_interval_, poll_interval_);
}

} // namespace collector::cloud
