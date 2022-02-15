#include <channel/test_channel.h>
#include <collector/kernel/cgroup_handler.h>
#include <collector/kernel/kernel_collector.h>
#include <common/host_info.h>
#include <common/intake_encoder.h>
#include <config/config_file.h>
#include <config/intake_config.h>
#include <generated/flowmill/ingest/otlp_log_encoder.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/authz_fetcher.h>
#include <util/aws_instance_metadata.h>
#include <util/boot_time.h>
#include <util/curl_engine.h>
#include <util/gcp_instance_metadata.h>
#include <util/json.h>
#include <util/log.h>
#include <util/logger.h>
#include <util/system_ops.h>

#include <sys/utsname.h>

#include <regex>
#include <string>
#include <unordered_map>

#include <uv.h>

extern "C" {
/* bpf source code */
extern char agent_bpf_c[];
extern unsigned int agent_bpf_c_len;
} // extern "C"

class TestIntakeConfig : public config::IntakeConfig {
  using config::IntakeConfig::IntakeConfig;

  bool allow_compression() const { return false; }

  std::unique_ptr<channel::NetworkChannel>
  make_channel(uv_loop_t &loop, std::string_view private_key, std::string_view certificate) const
  {
    return std::make_unique<channel::TestChannel>(loop, encoder());
  }
};

// Conditions to be met before stopping test
struct StopConditions {
  u64 num_sends;
  std::vector<std::pair<std::string, u64>> names_and_counts;
  std::chrono::seconds timeout_sec;
};

class KernelCollectorTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    LOG::init(true); // log to console
    spdlog::set_level(spdlog::level::debug);
    ASSERT_EQ(0, uv_loop_init(&loop_));
  }

  void TearDown() override
  {
    stop_workload();

    print_message_counts();
    print_json_messages();

    ASSERT_EQ(false, timeout_exceeded_);
    ASSERT_EQ(0, get_test_channel()->get_num_failed_sends());
  }

  void start_kernel_collector(
      IntakeEncoder intake_encoder, StopConditions const &stop_conditions, std::string const &bpf_dump_file = "")
  {
    // This mostly duplicates the KernelCollector setup done in flowmill-collector/collector/kernel/main.cc.

    /* Read our BPF program*/
    /* resolve includes */
    bpf_src_ = std::string((char *)agent_bpf_c, agent_bpf_c_len);

    u64 boot_time_adjustment = get_boot_time();
    /* insert time onto the bpf program */
    bpf_src_ = std::regex_replace(bpf_src_, std::regex("BOOT_TIME_ADJUSTMENT"), fmt::format("{}uLL", boot_time_adjustment));
    bpf_src_ = std::regex_replace(bpf_src_, std::regex("FILTER_NS"), fmt::format("{}", 10 * 1000 * 1000ull));
    bpf_src_ = std::regex_replace(bpf_src_, std::regex("MAX_PID"), *read_file_as_string(MAX_PID_PROC_PATH).try_raise());
    bpf_src_ = std::regex_replace(bpf_src_, std::regex("CPU_MEM_IO_ENABLED"), std::string("0"));
    bpf_src_ = std::regex_replace(bpf_src_, std::regex("REPORT_DEBUG_EVENTS_PLACEHOLDER"), std::string("0"));

    test_intake_config_ = TestIntakeConfig(
        "",
        "",
        "",
        std::nullopt,
        {} /* record_output_path */,
        true /* disable_tls */,
        intake_encoder,
        collector::AuthMethod::none);

    auto const aws_metadata = AwsMetadata::fetch(1000ms);

    auto const gcp_metadata = GcpInstanceMetadata::fetch(1000ms);

    config::ConfigFile configuration_data(config::ConfigFile::YamlFormat(), "");

    std::unique_ptr<CurlEngine> curl_engine = CurlEngine::create(&loop_);

    std::optional<AuthzFetcher> authz_fetcher;

    bool const enable_http_metrics = true;

    bool const enable_userland_tcp = false;

    u64 const socket_stats_interval_sec = 10;

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
        .kernel_headers_source = KernelHeadersSource::unknown,
        .kernel_version = unamebuf.release,
        .hostname = hostname};

    kernel_collector_.emplace(
        bpf_src_,
        *test_intake_config_,
        boot_time_adjustment,
        aws_metadata.try_value(),
        gcp_metadata.try_value(),
        configuration_data.labels(),
        loop_,
        *curl_engine,
        authz_fetcher,
        enable_http_metrics,
        enable_userland_tcp,
        socket_stats_interval_sec,
        CgroupHandler::CgroupSettings{false, false},
        bpf_dump_file,
        host_info,
        EntrypointError::none);

    run_test_stopper(stop_conditions);
    run_workload_starter();

    LOG::info("starting event loop...");
    uv_run(&loop_, UV_RUN_DEFAULT);
  }

  void stop_kernel_collector() {
    kernel_collector_->on_close();
    uv_stop(&loop_);
  }

  void run_test_stopper(StopConditions const &stop_conditions)
  {
    auto stop_test_check = [&]() {
      if (stopwatch_) {
        timeout_exceeded_ = stopwatch_->elapsed(stop_conditions.timeout_sec);
        LOG::trace(
            "stop_test_check() stop_conditions.timeout_sec {} exceeded {}", stop_conditions.timeout_sec, timeout_exceeded_);
        if (timeout_exceeded_) {
          LOG::error("stop_test_check() test timeout of {} exceeded", stop_conditions.timeout_sec);
          stop_kernel_collector();
          return;
        }
      }

      auto channel = get_test_channel();
      auto num_sends = channel->get_num_sends();
      LOG::trace(
          "stop_test_check() channel->get_num_sends() = {} stop_conditions.num_sends = {}",
          num_sends,
          stop_conditions.num_sends);
      if (num_sends < stop_conditions.num_sends) {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }

      auto &message_counts = channel->get_message_counts();
      bool reschedule = false;
      for (auto const &[name, count] : stop_conditions.names_and_counts) {
        auto message_count = message_counts[name];
        LOG::trace("stop_test_check() message_counts[{}] = {} stop count = {}", name, message_count, count);
        if (message_count < count) {
          reschedule = true;
        }
      }
      if (reschedule) {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }

      stop_kernel_collector();
    };

    stop_test_timer_ = std::make_unique<scheduling::Timer>(loop_, stop_test_check);
    stop_test_timer_->defer(std::chrono::seconds(1));
  }

  void start_workload()
  {
    thr1_ = std::thread([]() {
      system(
          "echo starting workload 1; exec 1> /tmp/workload1.log 2>&1; set -x; whoami; pwd; ls; cd /tmp; pwd; ls; cd /; pwd; ls; cd ~; pwd; ls; echo workload 1 complete");
    });
    thr2_ = std::thread([]() {
      system(
          "echo starting workload 2; /root/src/test/workload/sockets/sockets.py 10 20 > /dev/null; echo workload 2 complete");
    });
  };

  void run_workload_starter()
  {
    auto &message_counts = get_test_channel()->get_message_counts();

    auto start_workload_check = [&]() {
      LOG::trace("in start_workload_check()");
      if ((message_counts["bpf_compiled"] >= 1) || (message_counts["socket_steady_state"] >= 1) ||
          (message_counts["process_steady_state"] >= 1)) {
        LOG::trace("start_workload_check() STARTING");
        start_workload();
        // this is where we start timing for purposes of the test timeout
        stopwatch_.emplace();
      } else {
        start_workload_timer_->defer(std::chrono::seconds(1));
      }
    };

    start_workload_timer_ = std::make_unique<scheduling::Timer>(loop_, start_workload_check);
    start_workload_timer_->defer(std::chrono::seconds(1));
  }

  void stop_workload()
  {
    if (thr1_.joinable()) {
      thr1_.join();
    }
    if (thr2_.joinable()) {
      thr2_.join();
    }
  };

  channel::TestChannel *get_test_channel()
  {
    return dynamic_cast<channel::TestChannel *>(kernel_collector_->primary_channel_.get());
  }

  void print_message_counts()
  {
    auto &message_counts = get_test_channel()->get_message_counts();
    LOG::debug("message_counts:");
    for (auto const &name_and_count : message_counts) {
      LOG::debug("message_counts[\"{}\"] = {}", name_and_count.first, name_and_count.second);
    }
  }

  void print_json_messages()
  {
    LOG::trace("json_messages:");
    auto const &json_messages = get_test_channel()->get_json_messages();
    for (auto const &object : json_messages) {
      LOG::trace("{}", log_waive(object.dump()));
    }
  }

  uv_loop_t loop_;

  std::string bpf_src_;
  std::optional<TestIntakeConfig> test_intake_config_;
  std::optional<KernelCollector> kernel_collector_;

  bool timeout_exceeded_ = false;
  std::optional<StopWatch<>> stopwatch_;
  std::unique_ptr<scheduling::Timer> stop_test_timer_;
  std::unique_ptr<scheduling::Timer> start_workload_timer_;

  std::thread thr1_, thr2_;
};

TEST_F(KernelCollectorTest, binary)
{
  StopConditions stop_conditions{
    .num_sends = 25,
    .names_and_counts = {
      {"bpf_compiled", 1},
      {"begin_telemetry", 1},
      {"close_sock_info", 100},
      {"cloud_platform", 1},
      {"metadata_complete", 1},
      {"new_sock_info", 100},
      {"os_info", 1},
      {"pid_close_info", 5},
      {"pid_info_create", 5},
      {"pid_set_comm", 5},
      {"process_steady_state", 1},
      {"set_cgroup", 5},
      {"set_command", 5},
      {"set_config_label", 1},
      {"set_node_info", 1},
      {"set_tgid", 5},
      {"socket_stats", 100},
      {"socket_steady_state", 1}},
    .timeout_sec = std::chrono::seconds(60)};

  start_kernel_collector(IntakeEncoder::binary, stop_conditions);
}

TEST_F(KernelCollectorTest, otlp_log)
{
  StopConditions stop_conditions{
      .num_sends = 1000,
      .names_and_counts =
          {{"close_sock_info", 100},
           {"cloud_platform", 1},
           {"metadata_complete", 1},
           {"new_sock_info", 100},
           {"os_info", 1},
           {"pid_close_info", 5},
           {"pid_info_create", 5},
           {"pid_set_comm", 5},
           {"process_steady_state", 1},
           {"set_cgroup", 5},
           {"set_command", 5},
           {"set_config_label", 1},
           {"set_node_info", 1},
           {"set_tgid", 5},
           {"socket_stats", 100},
           {"socket_steady_state", 1}},
      .timeout_sec = std::chrono::seconds(60)};

  start_kernel_collector(IntakeEncoder::otlp_log, stop_conditions);
}
