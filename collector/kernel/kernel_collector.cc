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

#include <collector/kernel/kernel_collector.h>
#include <collector/kernel/kernel_collector_restarter.h>

#include <channel/tcp_channel.h>
#include <collector/component.h>
#include <collector/constants.h>
#include <collector/kernel/troubleshooting.h>
#include <collector/server_command.h>
#include <common/client_type.h>
#include <common/cloud_platform.h>
#include <common/collector_status.h>
#include <platform/userspace-time.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/resource_usage_reporter.h>

#include <absl/strings/match.h>

#include <netdb.h>

#include <random>

namespace {
static constexpr std::chrono::milliseconds TRY_CONNECTING_TIMEOUT = 5s;
static constexpr u64 connection_timeout_ms_ = 10000;
static constexpr u64 probe_holdoff_timeout_ms_ = 2000;
static constexpr u64 polling_timeout_ms_ = 100;
static constexpr u64 slow_polling_timeout_ms_ = 1000;

/* minimum time allowed between probes */
static constexpr u64 inter_probe_time_ns_ = 120 * 1000 * 1000 * 1000ul;
/* max jitter added to time between probes */
static constexpr std::chrono::milliseconds MAX_JITTER_TIME = 10s;
} // namespace

void __try_connecting_cb(uv_timer_t *timer)
{
  KernelCollector *collector = (KernelCollector *)timer->data;
  collector->try_connecting(timer);
}

void __connection_timeout_cb(uv_timer_t *timer)
{
  LOG::trace("KernelCollector: connection timeout");

  KernelCollector *collector = (KernelCollector *)timer->data;
  collector->connection_timeout(timer);
}

void __probe_holdoff_cb(uv_timer_t *timer)
{
  LOG::trace("KernelCollector: probe holdoff timeout");

  KernelCollector *collector = (KernelCollector *)timer->data;
  collector->probe_holdoff_timeout(timer);
}

void __polling_steady_state_cb(uv_timer_t *timer)
{
  KernelCollector *collector = (KernelCollector *)timer->data;
  collector->polling_steady_state(timer);
}

void __polling_steady_state_slow_cb(uv_timer_t *timer)
{
  KernelCollector *collector = (KernelCollector *)timer->data;
  collector->polling_steady_state_slow(timer);
}

void __handle_close_cb(uv_handle_t *handle)
{
  LOG::trace("KernelCollector: closed handle");
}

KernelCollector::KernelCollector(
    const std::string &full_program,
    config::IntakeConfig const &intake_config,
    u64 boot_time_adjustment,
    AwsMetadata const *aws_metadata,
    GcpInstanceMetadata const *gcp_metadata,
    std::map<std::string, std::string> configuration_data,
    uv_loop_t &loop,
    CurlEngine &curl_engine,
    std::optional<AuthzFetcher> &authz_fetcher,
    bool enable_http_metrics,
    bool enable_userland_tcp,
    u64 socket_stats_interval_sec,
    CgroupHandler::CgroupSettings cgroup_settings,
    ProcessHandler::CpuMemIoSettings const *cpu_mem_io_settings,
    std::string const &bpf_dump_file,
    HostInfo host_info,
    EntrypointError entrypoint_error)
    : full_program_(full_program),
      intake_config_(std::move(intake_config)),
      boot_time_adjustment_(boot_time_adjustment),
      aws_metadata_(aws_metadata),
      gcp_metadata_(gcp_metadata),
      configuration_data(configuration_data),
      host_info_(std::move(host_info)),
      entrypoint_error_(entrypoint_error),
      loop_(loop),
      last_lost_count_(0),
      encoder_(intake_config_.make_encoder()),
      callbacks_(*this),
      primary_channel_(intake_config_.make_channel(loop)),
      secondary_channel_(intake_config_.create_output_record_file()),
      upstream_connection_(
          WRITE_BUFFER_SIZE,
          intake_config_.allow_compression(),
          *primary_channel_,
          secondary_channel_ ? &secondary_channel_ : nullptr),
      writer_(upstream_connection_.buffered_writer(), monotonic, boot_time_adjustment, encoder_.get()),
      last_probe_monotonic_time_ns_(monotonic() - inter_probe_time_ns_),
      is_connected_(false),
      curl_engine_(curl_engine),
      authz_fetcher_(authz_fetcher),
      heartbeat_sender_(
          loop_,
          [this] {
            send_heartbeat();
            return scheduling::JobFollowUp::ok;
          }),
      enable_http_metrics_(enable_http_metrics),
      enable_userland_tcp_(enable_userland_tcp),
      socket_stats_interval_sec_(socket_stats_interval_sec),
      cgroup_settings_(std::move(cgroup_settings)),
      cpu_mem_io_settings_(cpu_mem_io_settings),
      log_(writer_),
      nic_poller_(writer_, log_),
      kernel_collector_restarter_(*this)
{
  if (intake_config_.auth_method() == collector::AuthMethod::authz) {
    assert(authz_fetcher_);
    authz_token_ = authz_fetcher_->token().value();
  }

  if (!bpf_dump_file.empty()) {
    auto const error = bpf_dump_file_.create(
        bpf_dump_file.c_str(),
        FileDescriptor::Access::write_only,
        FileDescriptor::Positioning::append,
        FileDescriptor::Permission::read_write,
        FileDescriptor::Permission::read);

    if (error) {
      LOG::warn("unable to open/create eBPF dump file at '{}': {}", bpf_dump_file, error);
    }
  }

  int res;
  /* initialize polling timer */
  res = uv_timer_init(&loop_, &polling_timer_);
  if (res != 0)
    throw std::runtime_error("Could not init polling_timer_");
  polling_timer_.data = this;

  /* initialize slow polling timer */
  res = uv_timer_init(&loop_, &slow_timer_);
  if (res != 0)
    throw std::runtime_error("Could not init slow_timer_");
  slow_timer_.data = this;

  /* initialize connection timeout timer */
  res = uv_timer_init(&loop_, &connection_timeout_);
  if (res != 0)
    throw std::runtime_error("Could not init connection_timeout_");
  connection_timeout_.data = this;

  /* initialize probe_holdoff_timeor */
  res = uv_timer_init(&loop_, &probe_holdoff_timer_);
  if (res != 0)
    throw std::runtime_error("Could not init probe_holdoff_timer_");
  probe_holdoff_timer_.data = this;

  /* initialize try_connecting timer */
  res = uv_timer_init(&loop_, &try_connecting_timer_);
  if (res != 0)
    throw std::runtime_error("Could not init try_connecting_timer_");
  try_connecting_timer_.data = this;

  /* start in try_connecting state */
  enter_try_connecting();
}

KernelCollector::~KernelCollector()
{
  on_close();
}

void KernelCollector::try_connecting(uv_timer_t *timer)
{
  cleanup_pointers();

  try {
    LOG::info("connecting to {}...", intake_config_);
    upstream_connection_.connect(callbacks_);
  } catch (std::exception &e) {
    LOG::trace("upstream connect threw exception: {}", e.what());
    return;
  }

  enter_connecting();
}

void KernelCollector::connection_timeout(uv_timer_t *timer)
{
  LOG::trace("connection timeout");
  upstream_connection_.close();
}

void KernelCollector::polling_steady_state(uv_timer_t *timer)
{
  if (disabled_) {
    return;
  }
  /* push data to server */
  try {
    bpf_handler_->start_poll(1, 1);
  } catch (std::exception &e) {
    log_.error("BPFHandler::start_poll threw exception '{}'", e.what());
    enter_try_connecting();
    return;
  }

  /* only print when some messages got lost */
  if (bpf_handler_->serv_lost_count() > last_lost_count_) {
    last_lost_count_ = bpf_handler_->serv_lost_count();
    LOG::trace("-polling- lost count: {}", bpf_handler_->serv_lost_count());
  }
}

void KernelCollector::polling_steady_state_slow(uv_timer_t *timer)
{
  if (disabled_) {
    return;
  }
  bpf_handler_->slow_poll();
  nic_poller_.poll();

  ResourceUsageReporter::report(writer_);
  upstream_connection_.flush();
}

void KernelCollector::on_close()
{
  cleanup_pointers();
  upstream_connection_.close();
  uv_close((uv_handle_t *)&polling_timer_, __handle_close_cb);
  uv_close((uv_handle_t *)&slow_timer_, __handle_close_cb);
  uv_close((uv_handle_t *)&connection_timeout_, __handle_close_cb);
  uv_close((uv_handle_t *)&probe_holdoff_timer_, __handle_close_cb);
  uv_close((uv_handle_t *)&try_connecting_timer_, __handle_close_cb);
}

void KernelCollector::on_upstream_connected()
{
  LOG::trace("upstream connected");

  try {
    send_connection_metadata();
  } catch (std::exception &e) {
    LOG::error("Exception thrown when sending authentication request and agent metadata: {}", e.what());
    return;
  }
}

void KernelCollector::on_authenticated()
{
  LOG::trace("Authenticated, entering probe hold-off");
  enter_probe_holdoff();

  heartbeat_sender_.start(HEARTBEAT_INTERVAL, HEARTBEAT_INTERVAL);
}

void KernelCollector::probe_holdoff_timeout(uv_timer_t *timer)
{
  auto const upstream_connection_flush_and_close = [this]() {
    upstream_connection_.flush();
    upstream_connection_.close();
  };

  if (entrypoint_error_ != EntrypointError::none) {
    print_troubleshooting_message_and_exit(host_info_, entrypoint_error_, log_, upstream_connection_flush_and_close);
    return;
  }

  LOG::trace("Adding probes");

  last_probe_monotonic_time_ns_ = monotonic();

  auto const handle_exception = [&](TroubleshootItem item, std::exception const &e) {
    log_.error("Exception during BPFHandler initialization, closing connection: {}", e.what());
    print_troubleshooting_message_and_exit(host_info_, item, e, log_, upstream_connection_flush_and_close);
  };

  auto potential_troubleshoot_item = TroubleshootItem::bpf_compilation_failed;
  try {
    bpf_handler_.emplace(
        loop_, full_program_, enable_http_metrics_, enable_userland_tcp_, bpf_dump_file_, log_, encoder_.get());

    potential_troubleshoot_item = TroubleshootItem::unexpected_exception;
    writer_.bpf_compiled();

    kernel_collector_restarter_.reset();
    bpf_handler_->load_buffered_poller(
        upstream_connection_.buffered_writer(),
        boot_time_adjustment_,
        curl_engine_,
        nic_poller_,
        socket_stats_interval_sec_,
        cgroup_settings_,
        cpu_mem_io_settings_,
        kernel_collector_restarter_);

    potential_troubleshoot_item = TroubleshootItem::bpf_load_probes_failed;
    bpf_handler_->load_probes(writer_);

    /* Start running buf_poller in steady-state */
    potential_troubleshoot_item = TroubleshootItem::unexpected_exception;
    writer_.begin_telemetry();
    writer_.collector_health(integer_value(::collector::CollectorStatus::healthy), 0);
    LOG::info("Agent connected successfully. Telemetry is flowing!");
    is_connected_ = true;

    kernel_collector_restarter_.startup_completed();

    enter_polling_state();
  } catch (std::system_error &e) {
    if (e.code().value() == EPERM) {
      handle_exception(TroubleshootItem::operation_not_permitted, e);
      return;
    }
    handle_exception(potential_troubleshoot_item, e);
    return;
  } catch (std::exception &e) {
    handle_exception(potential_troubleshoot_item, e);
    return;
  }
}

void KernelCollector::send_connection_metadata()
{
  // send a version_info message
  upstream_connection_.set_compression(false);
  writer_.version_info(versions::release.major(), versions::release.minor(), versions::release.build());
  upstream_connection_.flush();
  upstream_connection_.set_compression(true);

  // we use strncpy in a way that might truncate the '\0' at the end, so need to
  // ask GCC (8.0+ not to fail)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"

  /* Write config file labels */
#define make_bufs_from_field(struct_name, field1, buf_name1, field2, buf_name2)                                                \
  struct struct_name __##struct_name##__##buf_name1;                                                                           \
  char buf_name1[sizeof(__##struct_name##__##buf_name1.field1)] = {};                                                          \
  struct struct_name __##struct_name##__##buf_name2;                                                                           \
  char buf_name2[sizeof(__##struct_name##__##buf_name2.field2)] = {};

  switch (intake_config_.auth_method()) {
  case collector::AuthMethod::none:
    if (intake_config_.encoder() == IntakeEncoder::binary) {
      writer_.no_auth_connect(static_cast<u8>(ClientType::kernel), jb_blob{host_info_.hostname});
      upstream_connection_.flush();
    }
    break;
  case collector::AuthMethod::authz: {
    assert(authz_token_);
    auto const &token = authz_token_->payload();
    LOG::info(
        "sending authz token with {}s left until expiration (iat={}s exp={}s)",
        authz_token_->time_left<std::chrono::seconds>(std::chrono::system_clock::now()).count(),
        authz_token_->issued_at<std::chrono::seconds>().count(),
        authz_token_->expiration<std::chrono::seconds>().count());
    writer_.authz_authenticate(jb_blob{token}, static_cast<u8>(ClientType::kernel), jb_blob{host_info_.hostname});
    upstream_connection_.flush();
  } break;
  default:
    throw std::runtime_error("invalid auth_method");
    break;
  }

  writer_.os_info(
      integer_value(host_info_.os), host_info_.os_flavor, jb_blob{host_info_.os_version}, jb_blob{host_info_.kernel_version});

  writer_.report_cpu_cores(std::thread::hardware_concurrency());

  writer_.kernel_headers_source(integer_value(host_info_.kernel_headers_source));

  if (entrypoint_error_ != EntrypointError::none) {
    writer_.entrypoint_error(integer_value(entrypoint_error_));
    upstream_connection_.flush();
  }

  for (auto const &label : configuration_data) {
    writer_.set_config_label(jb_blob{label.first}, jb_blob{label.second});
  }

  /* Kernel version */
  constexpr std::string_view kernel_version_label = "__kernel_version";
  writer_.set_config_label(jb_blob{kernel_version_label}, jb_blob{host_info_.kernel_version});
  upstream_connection_.flush();

#define make_buf_from_field(struct_name, field, buf_name)                                                                      \
  struct struct_name __##struct_name##__##buf_name;                                                                            \
  char buf_name[sizeof(__##struct_name##__##buf_name.field)] = {};

  if (aws_metadata_) {
    writer_.cloud_platform(static_cast<u16>(CloudPlatform::aws));
    if (auto const &account_id = aws_metadata_->account_id()) {
      LOG::trace_in(
          std::make_tuple(CloudPlatform::aws, collector::Component::auth), "reporting aws account id: {}", account_id.value());
      writer_.cloud_platform_account_info(jb_blob{account_id.value()});
    } else {
      LOG::trace_in(std::make_tuple(CloudPlatform::aws, collector::Component::auth), "no aws account id to report");
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

    upstream_connection_.flush();

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
    // LOG::trace_in(
    //   std::make_tuple(CloudPlatform::gcp, collector::Component::auth),
    //   "reporting gcp account id: {}", account_id.value()
    // );
    // writer_.cloud_platform_account_info(jb_blob{account_id});

    writer_.set_node_info(
        jb_blob{gcp_metadata_->az()},
        jb_blob{gcp_metadata_->role()},
        jb_blob{gcp_metadata_->hostname()},
        jb_blob{gcp_metadata_->type()});

    upstream_connection_.flush();

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

    writer_.set_node_info(jb_blob{/* az */}, jb_blob{/* role */}, jb_blob{host_info_.hostname}, jb_blob{/* instance_type */});

    // no network interface data (public/private ip) to send
  }

  // pop the "-Wstringop-truncation" warning quelch
#pragma GCC diagnostic pop

  /* Finished sending metadata */
  writer_.metadata_complete(0);

  // Fake DNS entry to classify Flowmill traffic
  if (auto const connected_address = upstream_connection_.connected_address()) {
    writer_.dns_response_fake(
        /*total_dn_len=*/8,
        /*ips=*/jb_blob{(char *)(&(connected_address)), (u16)(sizeof(in_addr_t))},
        /*dn=*/jb_blob{"flowmill", 8});
  }

  upstream_connection_.flush();

  on_authenticated();
}

void KernelCollector::on_error(int error)
{
  /* we don't want attempts to perform IO on the channel */
  heartbeat_sender_.stop();
  stop_all_timers();
}

void KernelCollector::restart()
{
  /* we don't want attempts to perform IO on the channel */
  heartbeat_sender_.stop();

  // flush the channel so previously sent messages make it to the pipeline server
  upstream_connection_.flush();

  // close the channel, it will trigger reconnect via KernelCollector::Callbacks::on_closed() which calls enter_try_connecting()
  upstream_connection_.close();
}

void KernelCollector::cleanup_pointers()
{
  bpf_handler_.reset();
}

void KernelCollector::received_data(const u8 *data, int data_len)
{
  std::string_view const response{reinterpret_cast<char const *>(data), static_cast<std::size_t>(data_len)};
  LOG::trace("KernelCollector::received_data({}): {}", data_len);

  switch (intake_config_.encoder()) {
  case IntakeEncoder::binary: {
    static constexpr int max_command_length = 8;

    for (int i = 0; i < data_len; i++) {
      received_command_ = (received_command_ << 8) | *(data + i);
      recieved_length_++;

      if (recieved_length_ == max_command_length) {
        handle_received_command(received_command_);
        recieved_length_ = 0;
      }
    }
    break;
  }

  case IntakeEncoder::otlp_log: {
    static constexpr std::string_view http_200 = "HTTP/1.1 200 ";
    if (response.size() < http_200.size() || response.substr(0, http_200.size()) != http_200) {
      LOG::error("HTTP response from OTLP log collector (sz:{}): {}", data_len, response);
    }
    break;
  }
  }
}

void KernelCollector::handle_received_command(u64 command)
{
  if (command == static_cast<u64>(ServerCommand::DISABLE_SEND)) {
    LOG::info("Stop sending data, instructed by the server.");
    disabled_ = true;
  }
}

void KernelCollector::send_heartbeat()
{
  writer_.heartbeat();
  upstream_connection_.flush();
}

KernelCollector::Callbacks::Callbacks(KernelCollector &collector) : collector_(collector) {}

u32 KernelCollector::Callbacks::received_data(const u8 *data, int data_len)
{
  collector_.received_data(data, data_len);
  return data_len;
}

void KernelCollector::Callbacks::on_error(int err)
{
  LOG::trace("upstream connection error {}", static_cast<std::errc>(-err));

  collector_.on_error(err);

  /* close the channel, it will trigger reconnect */
  collector_.upstream_connection_.close();
}

void KernelCollector::Callbacks::on_closed()
{
  LOG::trace("closed upstream connection, will reconnect...");
  StopWatch<> refresh_time;
  if (collector_.intake_config_.auth_method() == collector::AuthMethod::authz) {
    assert(collector_.authz_fetcher_);
    if (auto const &current = collector_.authz_fetcher_->token();
        !current || current->has_expired(std::chrono::system_clock::now())) {
      LOG::trace("refreshing invalid or expired authz token before reconnecting...");
      collector_.authz_fetcher_->sync_refresh();
    }
  }
  collector_.enter_try_connecting(refresh_time.elapsed<std::chrono::milliseconds>());
}

void KernelCollector::Callbacks::on_connect()
{
  LOG::trace("established upstream connection");
  collector_.on_upstream_connected();
}

void KernelCollector::enter_try_connecting(std::chrono::milliseconds discount)
{
  if (is_connected_) {
    LOG::info("disconnected from upstream, attempting to reconnect...");
    is_connected_ = false;
  }

  stop_all_timers();

  auto timeout = discount > TRY_CONNECTING_TIMEOUT ? 0ms : TRY_CONNECTING_TIMEOUT - discount;
  u64 now = monotonic();

  if (now - last_probe_monotonic_time_ns_ < inter_probe_time_ns_) {
    // need to bound using inter_probe_time_ns_
    std::chrono::milliseconds const at_least{
        (inter_probe_time_ns_ - (now - last_probe_monotonic_time_ns_)) / ((u64)1000 * 1000)};
    timeout = std::max(timeout, at_least);
  }

  // add jitter
  {
    std::random_device rd;
    std::uniform_int_distribution<u64> d(0, integer_time<std::chrono::milliseconds>(MAX_JITTER_TIME));
    timeout += std::chrono::milliseconds{d(rd)};
  }

  LOG::trace(
      "KernelCollector: entering TRY_CONNECTING state. Next "
      "reconnection attempt in {}",
      timeout);
  int res = uv_timer_start(
      &try_connecting_timer_,
      __try_connecting_cb,
      integer_time<std::chrono::milliseconds>(timeout),
      integer_time<std::chrono::milliseconds>(TRY_CONNECTING_TIMEOUT));
  if (res != 0) {
    throw std::runtime_error("Could not start try_connecting_timer");
  }
}

void KernelCollector::enter_connecting()
{
  stop_all_timers();

  LOG::trace("KernelCollector: entering CONNECTING state");

  int res = uv_timer_start(&connection_timeout_, __connection_timeout_cb, connection_timeout_ms_, 0);
  if (res != 0)
    throw std::runtime_error("Could not start connection_timeout timer");
}

void KernelCollector::enter_probe_holdoff()
{
  stop_all_timers();

  LOG::trace("KernelCollector: entering PROBE_HOLDOFF state");

  int res = uv_timer_start(&probe_holdoff_timer_, __probe_holdoff_cb, probe_holdoff_timeout_ms_, 0);
  if (res != 0)
    throw std::runtime_error("Could not start probe_holdoff_timer");
}

void KernelCollector::enter_polling_state()
{
  stop_all_timers();

  LOG::trace("KernelCollector: entering POLLING state");

  int res = uv_timer_start(&polling_timer_, __polling_steady_state_cb, polling_timeout_ms_, polling_timeout_ms_);
  if (res != 0)
    throw std::runtime_error("Could not start polling_timer");

  res = uv_timer_start(&slow_timer_, __polling_steady_state_slow_cb, slow_polling_timeout_ms_, slow_polling_timeout_ms_);
  if (res != 0)
    throw std::runtime_error("Could not start slow_timer");
}

void KernelCollector::stop_all_timers()
{
  uv_timer_stop(&try_connecting_timer_);
  uv_timer_stop(&connection_timeout_);
  uv_timer_stop(&probe_holdoff_timer_);
  uv_timer_stop(&polling_timer_);
  uv_timer_stop(&slow_timer_);
}

std::chrono::milliseconds KernelCollector::update_authz_token(AuthzToken const &token)
{
  assert(authz_token_);
  auto const time_left = std::chrono::duration_cast<std::chrono::milliseconds>(
      authz_token_->expiration() - std::chrono::system_clock::now().time_since_epoch());
  authz_token_ = token;
  return time_left;
}

#ifndef NDEBUG
void KernelCollector::debug_bpf_lost_samples()
{
  if (bpf_handler_) {
    bpf_handler_->debug_bpf_lost_samples();
  }
}
#endif
