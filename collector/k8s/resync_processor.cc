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

#include <collector/k8s/resync_processor.h>

#include <channel/reconnecting_channel.h>
#include <common/collector_status.h>
#include <util/boot_time.h>
#include <util/element_queue_cpp.h>
#include <util/log.h>
#include <util/resource_usage_reporter.h>

#include <cstring>

namespace collector {
namespace {
void poll_timer_cb(uv_timer_t *timer)
{
  ResyncProcessor *processor = (ResyncProcessor *)(timer->data);
  processor->poll();
}

u64 extract_u64(const char *buf)
{
  u64 n = 0;
  memcpy(&n, buf, 8);
  return n;
}

} // namespace

ResyncProcessor::ResyncProcessor(
    uv_loop_t &loop,
    ResyncQueueConsumerInterface *resync_queue,
    channel::ReconnectingChannel &reconnecting_channel,
    config::ConfigFile const &configuration_data,
    std::string_view hostname,
    AuthzFetcher &authz_fetcher,
    std::chrono::milliseconds aws_metadata_timeout,
    std::chrono::seconds heartbeat_interval,
    std::size_t write_buffer_size)
    : loop_(loop),
      resync_queue_(resync_queue),
      reconnecting_channel_(reconnecting_channel),
      encoder_(reconnecting_channel_.intake_config().make_encoder()),
      writer_(reconnecting_channel_.buffered_writer(), monotonic, get_boot_time(), encoder_.get()),
      caretaker_(
          hostname,
          ClientType::k8s,
          authz_fetcher,
          configuration_data.labels(),
          &loop,
          writer_,
          aws_metadata_timeout,
          heartbeat_interval,
          std::bind(&channel::ReconnectingChannel::flush, &reconnecting_channel_),
          std::bind(&channel::ReconnectingChannel::set_compression, &reconnecting_channel_, std::placeholders::_1),
          std::bind(&ResyncProcessor::on_authenticated, this)),
      active_resync_(0),
      dirty_(false)
{
  int res = uv_timer_init(&loop_, &poll_timer_);
  if (res != 0) {
    throw std::runtime_error("Cannot init poll_timer");
  }
  poll_timer_.data = this;

  start_poll_timer();
}

ResyncProcessor::~ResyncProcessor()
{
  stop_poll_timer();

  uv_close((uv_handle_t *)&poll_timer_, NULL);
}

void ResyncProcessor::start_poll_timer()
{
  int res = uv_timer_start(&poll_timer_, poll_timer_cb, poll_interval_ms_, 0);

  if (res != 0) {
    LOG::error("Cannot start poll_timer: {}", uv_err_name(res));
  }
}

void ResyncProcessor::stop_poll_timer()
{
  uv_timer_stop(&poll_timer_);
}

void ResyncProcessor::poll()
{
  start_poll_timer();

  poll_count_++;
  // One heart beat per every 5 miniutes
  if ((poll_count_ % (5 * 60 * 1000 / poll_interval_ms_) == 0)) {
    LOG::info(
        "Poll loop, connected: {},  resync: {}, message_count: {}",
        reconnecting_channel_.is_open(),
        active_resync_,
        message_count_);
  }

  for (int i = 0; i < num_iterations_per_poll_; i++) {
    u64 resync_queue_last_resync = resync_queue_->consumer_get_last_resync();

    if (active_resync_ < resync_queue_last_resync) {
      active_resync_ = resync_queue_last_resync;

      if (dirty_) {
        dirty_ = false;
        if (reconnecting_channel_.is_open()) {
          LOG::warn("Send resync command to sever due to k8s-watch reset.");
          writer_.pod_resync(active_resync_);
          reconnecting_channel_.flush();
        }
      }
    }

    auto *queue = resync_queue_->consumer_get_queue();
    queue->start_read_batch();
    while (queue->peek() > 0) {
      char *read_buf = nullptr;
      int element_length = queue->read(read_buf);

      if (element_length <= 8) {
        LOG::warn("ResyncProcessor is in incorrect state.");
        ;
        queue->finish_read_batch();
        if (reconnecting_channel_.is_open() && dirty_) {
          writer_.pod_resync(active_resync_);
          reconnecting_channel_.flush();
        }
        if (dirty_) {
          active_resync_ = resync_queue_->consumer_get_last_resync() + 1;
          dirty_ = false;
        }
        resync_queue_->consumer_reset();
        return;
      }

      u64 element_resync = extract_u64(read_buf);
      if (element_resync < active_resync_) {
        LOG::trace("Ignorning older element {} vs. active {}", element_resync, active_resync_);
        continue;
      }

      if (element_resync > active_resync_) {
        u64 resync_queue_last_resync = resync_queue_->consumer_get_last_resync();
        active_resync_ = resync_queue_last_resync;
        if (dirty_) {
          dirty_ = false;
          if (reconnecting_channel_.is_open()) {
            LOG::warn("Send resync command to sever due to k8s-watch reset.");
            writer_.pod_resync(active_resync_);
            reconnecting_channel_.flush();
          }
        }
      }
      dirty_ = true;
      message_count_++;
      if (reconnecting_channel_.is_open()) {
        reconnecting_channel_.send((const u8 *)(read_buf + 8), element_length - 8);
      }
    }
    queue->finish_read_batch();
  } // for (int i ...)

  writer_.collector_health(integer_value(::collector::CollectorStatus::healthy), 0);

  ResourceUsageReporter::report(writer_);

  reconnecting_channel_.flush();
}

void ResyncProcessor::on_error(int err)
{
  caretaker_.set_disconnected();
}

void ResyncProcessor::on_connect()
{
  caretaker_.set_connected();
}

void ResyncProcessor::on_authenticated()
{
  message_count_ = 0;
  if (dirty_) {
    LOG::warn("Reset collector due to remote connection reset.");

    resync_queue_->consumer_reset();
    active_resync_++;
    dirty_ = false;
  }
}

} // namespace collector
