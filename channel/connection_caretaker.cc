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

#include <channel/connection_caretaker.h>

#include <common/cloud_platform.h>
#include <common/constants.h>
#include <util/boot_time.h>
#include <util/log.h>
#include <util/log_formatters.h>

#include <absl/strings/match.h>

namespace channel {

namespace {

void heartbeat_timer_cb(uv_timer_t *timer)
{
  auto *caretaker = (ConnectionCaretaker *)(timer->data);
  caretaker->send_heartbeat();
}

} // namespace

ConnectionCaretaker::ConnectionCaretaker(
    std::string_view hostname,
    ClientType client_type,
    config::ConfigFile::LabelsMap const &config_data,
    uv_loop_t *loop,
    flowmill::ingest::Writer &writer,
    std::chrono::milliseconds metadata_timeout,
    std::chrono::milliseconds heartbeat_interval,
    std::function<void()> flush_cb,
    std::function<void(bool)> set_compression_cb,
    std::function<void()> on_connected_cb)
    : hostname_(hostname),
      client_type_(client_type),
      config_data_(config_data),
      loop_(loop),
      heartbeat_interval_(heartbeat_interval),
      flush_cb_(std::move(flush_cb)),
      set_compression_cb_(std::move(set_compression_cb)),
      on_connected_cb_(std::move(on_connected_cb)),
      writer_(writer)
{
  assert(loop != nullptr);
  assert(heartbeat_interval_.count() > 0);

  LOG::trace_in(CloudPlatform::aws, "--- resolving AWS metadata ---");
  if (auto aws_metadata = AwsMetadata::fetch(metadata_timeout)) {
    aws_metadata_.emplace(std::move(aws_metadata.value()));
    aws_metadata_->print_instance_metadata();
    aws_metadata_->print_interfaces();
  } else {
    LOG::warn("Unable to fetch AWS metadata: {}", aws_metadata.error());
  }

  LOG::trace_in(CloudPlatform::gcp, "--- resolving GCP metadata ---");
  if (auto gcp_metadata = GcpInstanceMetadata::fetch(metadata_timeout)) {
    gcp_metadata_.emplace(std::move(gcp_metadata.value()));
    gcp_metadata_->print();
  } else {
    LOG::warn("Unable to fetch GCP metadata: {}", gcp_metadata.error());
  }

  int res = uv_timer_init(loop_, &heartbeat_timer_);
  if (res != 0) {
    throw std::runtime_error("Cannot init heartbeat_timer");
  }
  heartbeat_timer_.data = this;
}

ConnectionCaretaker::~ConnectionCaretaker()
{
  stop_heartbeat();
  uv_close((uv_handle_t *)&heartbeat_timer_, NULL);
}

void ConnectionCaretaker::send_metadata_header()
{
  set_compression_cb_(false);
  LOG::info("initiating connection of {} collector version {}", client_type_, versions::release);
  writer_.version_info(versions::release.major(), versions::release.minor(), versions::release.build());
  flush();
  set_compression_cb_(true);

  writer_.connect(static_cast<u8>(client_type_), jb_blob(hostname_));

  writer_.report_cpu_cores(std::thread::hardware_concurrency());

  flush();

#define make_buf_from_field(struct_name, field, buf_name)                                                                      \
  struct struct_name __##struct_name##__##buf_name;                                                                            \
  char buf_name[sizeof(__##struct_name##__##buf_name.field)] = {};

  for (auto const &label : config_data_) {
    writer_.set_config_label(jb_blob{label.first}, jb_blob{label.second});
  }
  flush();

  if (aws_metadata_) {
    writer_.cloud_platform(static_cast<u16>(CloudPlatform::aws));
    if (auto const &account_id = aws_metadata_->account_id()) {
      LOG::trace_in(CloudPlatform::aws, "reporting aws account id: {}", account_id.value());
      writer_.cloud_platform_account_info(jb_blob{account_id.value()});
    } else {
      LOG::trace_in(CloudPlatform::aws, "no aws account id to report");
    }

    auto id = aws_metadata_->id().value();
    if (absl::StartsWith(id, std::string_view("i-"))) {
      id.remove_prefix(2);
    }

    writer_.set_node_info(
        jb_blob{aws_metadata_->az().value()},
        jb_blob{aws_metadata_->iam_role().value()},
        jb_blob{id},
        jb_blob{aws_metadata_->type().value()});
    flush();

    for (auto const &interface : aws_metadata_->network_interfaces()) {
      for (auto const &ipv4 : interface.private_ipv4s()) {
        struct sockaddr_in private_sa;
        int res = inet_pton(AF_INET, ipv4.c_str(), &(private_sa.sin_addr));
        if (res != 1) {
          continue;
        }
        make_buf_from_field(jb_ingest__private_ipv4_addr, vpc_id, vpc_id_buf);
        strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
        writer_.private_ipv4_addr(private_sa.sin_addr.s_addr, (u8 *)vpc_id_buf);
      }

      for (auto const &ipv6 : interface.ipv6s()) {
        struct sockaddr_in6 sa;
        int res = inet_pton(AF_INET6, ipv6.c_str(), &(sa.sin6_addr));
        if (res != 1) {
          continue;
        }
        make_buf_from_field(jb_ingest__ipv6_addr, vpc_id, vpc_id_buf);
        strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
        writer_.ipv6_addr(sa.sin6_addr.s6_addr, (u8 *)vpc_id_buf);
      }

      for (auto const &mapped_ipv4 : interface.mapped_ipv4s()) {
        struct sockaddr_in public_sa;
        int res = inet_pton(AF_INET, mapped_ipv4.first.c_str(), &(public_sa.sin_addr));
        if (res != 1) {
          continue;
        }
        struct sockaddr_in private_sa;
        res = inet_pton(AF_INET, mapped_ipv4.second.c_str(), &(private_sa.sin_addr));
        if (res != 1) {
          continue;
        }
        make_buf_from_field(jb_ingest__public_to_private_ipv4, vpc_id, vpc_id_buf);
        strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
        writer_.public_to_private_ipv4(public_sa.sin_addr.s_addr, private_sa.sin_addr.s_addr, (u8 *)vpc_id_buf);
      }
    }
  } else if (gcp_metadata_) {
    writer_.cloud_platform(static_cast<u16>(CloudPlatform::gcp));
    // TODO: obtain account_id for GCP and uncomment below
    // writer_.cloud_platform_account_info(jb_blob{account_id});

    writer_.set_node_info(
        jb_blob{gcp_metadata_->az()},
        jb_blob{gcp_metadata_->role()},
        jb_blob{gcp_metadata_->hostname()},
        jb_blob{gcp_metadata_->type()});
    flush();

    for (auto const &interface : gcp_metadata_->network_interfaces()) {
      if (auto const ipv4 = interface.ipv4()) {
        make_buf_from_field(jb_ingest__private_ipv4_addr, vpc_id, vpc_id_buf);
        strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
        writer_.private_ipv4_addr(ipv4->as_int(), (u8 *)vpc_id_buf);

        for (auto const &public_ip : interface.public_ips()) {
          make_buf_from_field(jb_ingest__public_to_private_ipv4, vpc_id, vpc_id_buf);
          strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
          writer_.public_to_private_ipv4(public_ip.as_int(), ipv4->as_int(), (u8 *)vpc_id_buf);
        }
      } else if (auto const ipv6 = interface.ipv6()) {
        uint8_t ipv6_buffer[16];
        ipv6->write_to(ipv6_buffer);
        make_buf_from_field(jb_ingest__ipv6_addr, vpc_id, vpc_id_buf);
        strncpy(vpc_id_buf, interface.vpc_id().c_str(), sizeof(vpc_id_buf));
        writer_.ipv6_addr(ipv6_buffer, (u8 *)vpc_id_buf);
      }
    }
  } else {
    writer_.cloud_platform(static_cast<u16>(CloudPlatform::unknown));

    writer_.set_node_info(jb_blob{/* az */}, jb_blob{/* role */}, jb_blob{hostname_}, jb_blob{/* instance_type */});

    // no network interface data (public/private ip) to send
  }

  writer_.metadata_complete(0);

  flush();
#undef make_buf_from_field

  on_connected_cb_();
}

void ConnectionCaretaker::flush()
{
  flush_cb_();
}

void ConnectionCaretaker::start_heartbeat()
{
  int res = uv_timer_start(&heartbeat_timer_, heartbeat_timer_cb, heartbeat_interval_.count(), heartbeat_interval_.count());

  if (res != 0) {
    LOG::error("Cannot start heartbeat_timer: {}", uv_err_name(res));
  }
}

void ConnectionCaretaker::stop_heartbeat()
{
  uv_timer_stop(&heartbeat_timer_);
}

void ConnectionCaretaker::send_heartbeat()
{
  LOG::debug("sending heartbeat for {} collector", client_type_);
  if (writer_.is_writable()) {
    writer_.heartbeat();
    flush();
  }
}

void ConnectionCaretaker::set_connected()
{
  LOG::info("collector {} connected to host", hostname_);
  send_metadata_header();
  start_heartbeat();
}

void ConnectionCaretaker::set_disconnected()
{
  LOG::info("collector {} disconnected from host", hostname_);
  stop_heartbeat();
}

} // namespace channel
