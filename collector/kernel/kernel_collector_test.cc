// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/test_channel.h>
#include <collector/kernel/cgroup_handler.h>
#include <collector/kernel/kernel_collector.h>
#include <common/host_info.h>
#include <common/intake_encoder.h>
#include <config/config_file.h>
#include <config/intake_config.h>
#include <generated/ebpf_net/ingest/meta.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <jitbuf/jb.h>
#include <spdlog/fmt/chrono.h>
#include <util/aws_instance_metadata.h>
#include <util/boot_time.h>
#include <util/code_timing.h>
#include <util/common_test.h>
#include <util/curl_engine.h>
#include <util/error_handling.h>
#include <util/gcp_instance_metadata.h>
#include <util/json.h>
#include <util/json_converter.h>
#include <util/log.h>
#include <util/log_whitelist.h>
#include <util/logger.h>
#include <util/system_ops.h>

#include <sys/utsname.h>

#include <map>
#include <regex>
#include <string>

#include <uv.h>

#define BPF_DUMP_FILE "/tmp/bpf-dump-file"
#define INTAKE_DUMP_FILE "/tmp/intake-dump-file"

extern "C" {
/* bpf source code */
extern char agent_bpf_c[];
extern unsigned int agent_bpf_c_len;
} // extern "C"

class TestIntakeConfig : public config::IntakeConfig {
  using config::IntakeConfig::IntakeConfig;

  bool allow_compression() const { return false; }

  std::unique_ptr<channel::NetworkChannel> make_channel(uv_loop_t &loop) const override
  {
    return std::make_unique<channel::TestChannel>(loop, encoder());
  }
};

// Conditions to be met before stopping test
struct StopConditions {
  std::chrono::seconds timeout_sec;
  u64 num_sends;
  std::map<std::string, u64> names_and_counts;
  bool wait_for_all_workloads_to_complete;
};

class KernelCollectorTest : public CommonTest {

protected:
  void SetUp() override
  {
    CommonTest::SetUp();

    // Allow relevant HTTP-related logs for this test
    set_log_whitelist<AgentLogKind>({AgentLogKind::HTTP, AgentLogKind::PROTOCOL, AgentLogKind::BPF, AgentLogKind::PERF});

    ASSERT_EQ(0, uv_loop_init(&loop_));
  }

  void TearDown() override
  {
    // Clean up loop_ to avoid valgrind and asan complaints about memory leaks.
    close_uv_loop_cleanly(&loop_);
  }

  void start_kernel_collector(
      IntakeEncoder intake_encoder,
      StopConditions const &stop_conditions,
      std::string const &bpf_dump_file = "",
      std::function<void(nlohmann::json const &)> const &ingest_msg_cb = {})
  {
    stop_conditions_.emplace(stop_conditions);

    // This mostly duplicates the KernelCollector setup done in collector/kernel/main.cc.

    // Create BPF configuration with test parameters
    u64 boot_time_adjustment = get_boot_time();

    test_intake_config_ = TestIntakeConfig("", "", INTAKE_DUMP_FILE, intake_encoder);

    auto const aws_metadata = AwsMetadata::fetch(1000ms);

    auto const gcp_metadata = GcpInstanceMetadata::fetch(1000ms);

    config::ConfigFile configuration_data(config::ConfigFile::YamlFormat(), "");

    std::unique_ptr<CurlEngine> curl_engine = CurlEngine::create(&loop_);

    bool const enable_http_metrics = true;

    bool const enable_userland_tcp = false;

    u64 const socket_stats_interval_sec = 10;

    BpfConfiguration bpf_config{
        .boot_time_adjustment = boot_time_adjustment,
        .filter_ns = 10 * 1000 * 1000ull,
        .enable_tcp_data_stream = enable_userland_tcp};

    struct utsname unamebuf;
    if (uname(&unamebuf)) {
      throw std::runtime_error("Failed to get system uname");
    }
    LOG::info(
        "Running on:\n"
        "   sysname: {}\n"
        "  nodename: {}\n"
        "   release: {}\n"
        "   version: {}\n"
        "   machine: {}",
        unamebuf.sysname,
        unamebuf.nodename,
        unamebuf.release,
        unamebuf.version,
        unamebuf.machine);

    // resolve hostname
    std::string const hostname = get_host_name(MAX_HOSTNAME_LENGTH).recover([&](auto &error) {
      LOG::error("Unable to retrieve host information from uname: {}", error);
      return aws_metadata->id().valid() ? std::string(aws_metadata->id().value()) : "(unknown)";
    });

    HostInfo host_info{
        .os = OperatingSystem::Linux,
        .os_flavor = integer_value(LinuxDistro::unknown),
        .os_version = "unknown",
        .kernel_headers_source = KernelHeadersSource::libbpf,
        .kernel_version = unamebuf.release,
        .hostname = hostname};

    kernel_collector_.emplace(
        bpf_config,
        *test_intake_config_,
        aws_metadata.try_value(),
        gcp_metadata.try_value(),
        configuration_data.labels(),
        loop_,
        *curl_engine,
        enable_http_metrics,
        socket_stats_interval_sec,
        CgroupHandler::CgroupSettings{false, std::nullopt},
        bpf_dump_file,
        host_info);

    if (ingest_msg_cb) {
      get_test_channel()->set_sent_msg_cb(ingest_msg_cb);
    }

    run_test_stopper();
    run_workload_starter();

    LOG::info("starting event loop...");
    uv_run(&loop_, UV_RUN_DEFAULT);
  }

  void stop_kernel_collector()
  {
    stop_workloads();

    print_json_messages();
    if (timeout_exceeded_) {
      print_stop_conditions();
    }
    print_message_counts();

    // NOTE: use EXPECT_s here because ASSERT_s fail fast, returning from the current function, skipping the cleanup below
    EXPECT_EQ(0ull, get_probe_handler().num_failed_probes_);
    EXPECT_TRUE(binary_messages_check_counts());
    EXPECT_EQ(0ull, get_test_channel()->get_num_failed_sends());
    EXPECT_EQ(false, timeout_exceeded_);

    auto &message_counts = get_test_channel()->get_message_counts();
    EXPECT_EQ(0ull, message_counts["bpf_log"]);

    kernel_collector_->on_close();

    uv_stop(&loop_);

    print_code_timings();
  }

  void run_test_stopper()
  {
    auto stop_test_check = [&]() {
      SCOPED_TIMING(StopTestCheck);

      auto const &stop_conditions = stop_conditions_->get();

      // check for test timeout
      if (stopwatch_) {
        timeout_exceeded_ = stopwatch_->elapsed(stop_conditions.timeout_sec);
        LOG::trace(
            "stop_test_check() stop_conditions timeout_sec {} exceeded {}", stop_conditions.timeout_sec, timeout_exceeded_);
        if (timeout_exceeded_) {
          LOG::error("stop_test_check() test timeout of {} exceeded", stop_conditions.timeout_sec);
          stop_kernel_collector();
          return;
        }
      }

      // wait for all workloads to complete if requested
      if (stop_conditions.wait_for_all_workloads_to_complete) {
        LOG::trace("stop_test_check() num_remaining_workloads_ = {}", num_remaining_workloads_);
        if (num_remaining_workloads_) {
          stop_test_timer_->defer(std::chrono::seconds(1));
          return;
        }
      }

      // check num_sends
      auto channel = get_test_channel();
      auto num_sends = channel->get_num_sends();
      LOG::trace(
          "stop_test_check() channel->get_num_sends() = {} stop_conditions num_sends = {}",
          num_sends,
          stop_conditions.num_sends);
      if (num_sends < stop_conditions.num_sends) {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }

      // check names_and_counts
      auto &message_counts = channel->get_message_counts();
      bool reschedule = false;
      for (auto const &[name, count] : stop_conditions.names_and_counts) {
        auto message_count = message_counts[name];
        LOG::trace("stop_test_check() message_counts[{}] = {}  \tstop count = {}", name, message_count, count);
        if (message_count < count) {
          reschedule = true;
        }
      }
      if (reschedule) {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }

      LOG::trace("stop_test_check() stop_conditions have been met - calling stop_kernel_collector()");
      stop_kernel_collector();
    };

    stop_test_timer_ = std::make_unique<scheduling::Timer>(loop_, stop_test_check);
    stop_test_timer_->defer(std::chrono::seconds(1));
  }

  void start_workload(std::function<void()> workload_cb)
  {
    auto index = workload_index_++;
    ++num_remaining_workloads_;

    auto workload_wrapper = [this, workload_cb, index]() {
      LOG::info("workload {} starting", index);
      workload_cb();
      LOG::info("workload {} complete", index);
      --num_remaining_workloads_;
    };

    workload_threads_.emplace_back(workload_wrapper);
  }

  void add_workload(std::function<void()> workload) { workloads_.push_back(std::move(workload)); }

  void add_workload_processes()
  {
    add_workload([]() {
      system(
          "exec 1> /tmp/workload-processes.log 2>&1; echo starting workload; set -x; whoami; pwd; ls; cd /tmp; pwd; ls; cd /; pwd; ls; cd ~; pwd; ls; echo workload complete");
    });
  }

  void add_workload_curl_otel()
  {
    add_workload([]() {
      system(
          "exec 1> /tmp/workload-curl-otel.log 2>&1; echo starting workload; for n in $(seq 1 10); do curl https://opentelemetry.io; done; echo workload complete");
    });
  }

  void add_workload_curl_localhost()
  {
    add_workload([]() {
      auto pid = fork();
      if (pid == 0) {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1); // redirect stdout
        dup2(fd, 2); // redirect stderr
        execl("/usr/bin/python3", "python3", "-m", "http.server", "28099", nullptr);
        exit(1);
      }

      system(
          "exec 1> /tmp/workload-curl-localhost.log 2>&1; echo starting workload; for n in $(seq 1 100); do curl localhost:28099; done; echo workload complete");

      kill(pid, SIGTERM);
    });
  }

  void add_workload_stress_ng_sock()
  {
    add_workload([]() {
      system(
          "exec 1> /tmp/workload-stress-ng-sock.log 2>&1; echo starting workload; for n in $(seq 1 30); do stress-ng --sock 2 --sock-domain ipv4 --sock-ops 2000 --sock-port 6787; sleep .1; done; echo workload complete");
    });
  }

  void start_workloads()
  {
    num_remaining_workloads_ = 0;

    for (auto workload : workloads_) {
      start_workload(workload);
    }
  };

  void run_workload_starter()
  {
    auto &message_counts = get_test_channel()->get_message_counts();

    auto start_workloads_check = [&]() {
      LOG::trace("in start_workloads_check()");
      if ((message_counts["bpf_compiled"] >= 1) && (message_counts["socket_steady_state"] >= 1) &&
          (message_counts["process_steady_state"] >= 1)) {
        LOG::trace("start_workloads_check() STARTING");
        start_workloads();
        // this is where we start timing for purposes of the test timeout
        stopwatch_.emplace();
      } else {
        start_workloads_timer_->defer(std::chrono::seconds(1));
      }
    };

    start_workloads_timer_ = std::make_unique<scheduling::Timer>(loop_, start_workloads_check);
    start_workloads_timer_->defer(std::chrono::seconds(1));
  }

  void stop_workloads()
  {
    for (auto &thr : workload_threads_) {
      if (thr.joinable()) {
        thr.join();
      }
    }
  };

  void print_stop_conditions()
  {
    auto &message_counts = get_test_channel()->get_message_counts();
    LOG::debug("stop conditions:");
    for (auto const &[name, count] : stop_conditions_->get().names_and_counts) {
      auto message_count = message_counts[name];
      LOG::debug(
          "stop_conditions[\"{}\"] = {}  \t({} received) {}",
          name,
          count,
          message_count,
          message_count < count ? " FAILED" : "");
    }
  }

  void print_message_counts()
  {
    LOG::debug("message_counts:");
    for (auto const &[name, count] : get_test_channel()->get_message_counts()) {
      LOG::debug("message_counts[\"{}\"] = {}", name, count);
    }
  }

  void print_json_messages()
  {
    LOG::trace("json_messages:");
    auto print_message = [&](channel::TestChannel::JsonMessageType const &msg) { LOG::trace("{}", log_waive(msg.dump())); };

    get_test_channel()->json_messages_for_each(print_message);
  }

  // This is an example of using TestChannel::binary_messages_for_each().  It looks at each message, counts the message type,
  // and compares the counts to TestChannel::message_counts_.
  bool binary_messages_check_counts()
  {
    channel::TestChannel::MessageCountsType check_message_counts;

    size_t num_binary_messages = 0;
    auto count_message = [&](channel::TestChannel::BinaryMessageType const &msg) {
      ++num_binary_messages;

      std::stringstream ss;
      json_converter::WireToJsonConverter<ebpf_net::ingest_metadata> converter(ss);

      converter.process(reinterpret_cast<char const *>(msg.data()), msg.size());
      std::string str = "[" + ss.str() + "]";
      nlohmann::json const objects = nlohmann::json::parse(str);
      for (auto const &object : objects) {
        ++check_message_counts[object["name"]];
      }
    };

    get_test_channel()->binary_messages_for_each(count_message);

    LOG::trace("check_message_counts:");
    for (auto const &[name, count] : check_message_counts) {
      LOG::trace("check_message_counts[\"{}\"] = {}", name, count);
    }

    return num_binary_messages ? check_message_counts == get_test_channel()->get_message_counts() : true;
  }

  channel::TestChannel *get_test_channel()
  {
    return dynamic_cast<channel::TestChannel *>(kernel_collector_->primary_channel_.get());
  }

  ProbeHandler &get_probe_handler()
  {
    if (!kernel_collector_->bpf_handler_) {
      throw std::runtime_error("std::optional bpf_handler_ does not have a value");
    }
    return kernel_collector_->bpf_handler_->probe_handler_;
  }

  uv_loop_t loop_;

  std::optional<TestIntakeConfig> test_intake_config_;
  std::optional<KernelCollector> kernel_collector_;

  bool timeout_exceeded_ = false;
  std::optional<StopWatch<>> stopwatch_;
  std::unique_ptr<scheduling::Timer> stop_test_timer_;
  std::unique_ptr<scheduling::Timer> start_workloads_timer_;

  std::vector<std::thread> workload_threads_;
  size_t workload_index_ = 0;
  std::atomic<size_t> num_remaining_workloads_ = std::numeric_limits<size_t>::max();
  std::vector<std::function<void()>> workloads_;

  std::optional<std::reference_wrapper<const StopConditions>> stop_conditions_;
};

// clang-format off
#define NAMES_AND_COUNTS_COMMON      \
  {"bpf_compiled", 1},               \
  {"begin_telemetry", 1},            \
  {"close_sock_info", 100},          \
  {"cloud_platform", 1},             \
  {"dns_response", 10},              \
  {"http_response", 10},             \
  {"metadata_complete", 1},          \
  {"new_sock_info", 100},            \
  {"os_info", 1},                    \
  {"pid_close_info", 5},             \
  {"pid_info_create", 5},            \
  {"pid_set_comm", 5},               \
  {"process_steady_state", 1},       \
  {"set_cgroup", 5},                 \
  {"set_command", 5},                \
  {"set_config_label", 1},           \
  {"set_node_info", 1},              \
  {"set_tgid", 5},                   \
  {"socket_stats", 100},             \
  {"socket_steady_state", 1},
// clang-format on

// Test basic kernel-collector functionality, validating that a minimum number of messages are seen as expected from running
// process and network workloads.
TEST_F(KernelCollectorTest, binary)
{
  StopConditions stop_conditions{
      .timeout_sec = std::chrono::seconds(60),
      .num_sends = 25,
      .names_and_counts = {NAMES_AND_COUNTS_COMMON},
      .wait_for_all_workloads_to_complete = true};

  add_workload_processes();
  add_workload_curl_otel();
  add_workload_curl_localhost();

  start_kernel_collector(IntakeEncoder::binary, stop_conditions, BPF_DUMP_FILE);
}

// This test was originally used to reproduce a race condition in the eBPF code that handles socket close events that would
// cause extraneous bpf_log messages by running a socket stress workload.
TEST_F(KernelCollectorTest, bpf_log)
{
  StopConditions stop_conditions{
      .timeout_sec = std::chrono::minutes(10),
      .num_sends = 1000,
      .names_and_counts = {},
      .wait_for_all_workloads_to_complete = true};

  add_workload_stress_ng_sock();

  // This will be called for each render message sent from the kernel-collector to 'ingest' (by the reducer in a real system)
  auto ingest_msg_cb = [&](nlohmann::json const &object) {
    SCOPED_TIMING(BpfLogTestIngestMsgCb);
    // Log any bpf_log messages so they are visible in CI output.
    if (object["name"] == "bpf_log") {
      LOG::error("bpf_log: {}", log_waive(object.dump()));
    }
  };

  start_kernel_collector(IntakeEncoder::binary, stop_conditions, BPF_DUMP_FILE, ingest_msg_cb);
}
