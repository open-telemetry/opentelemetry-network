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

#include <channel/component.h>
#include <collector/agent_log.h>
#include <collector/constants.h>
#include <collector/kernel/cgroup_handler.h>
#include <collector/kernel/kernel_collector.h>
#include <collector/kernel/troubleshooting.h>
#include <common/cloud_platform.h>
#include <config/config_file.h>
#include <platform/platform.h>
#include <util/agent_id.h>
#include <util/args_parser.h>
#include <util/boot_time.h>
#include <util/curl_engine.h>
#include <util/debug.h>
#include <util/environment_variables.h>
#include <util/file_ops.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/nomad_metadata.h>
#include <util/signal_handler.h>
#include <util/system_ops.h>
#include <util/utility.h>

#include <config.h>

#include <curlpp/cURLpp.hpp>

#include <dirent.h>
#include <linux/limits.h> /* PATH_MAX*/
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <uv.h>

#include <fstream>
#include <regex>
#include <sstream>

static constexpr size_t AGENT_MAX_MEMORY = 300ULL * 1024 * 1024; /* 300 MiB */

/* agent nice(2) value: -20 highest priority, 19 lowest, 0 default */
static constexpr int AGENT_NICE_VALUE = 5;

extern "C" {
/* bpf source code */
extern char agent_bpf_c[];
extern unsigned int agent_bpf_c_len;
} // extern "C"

static constexpr auto FLOWMILL_EXPORT_BPF_SRC_FILE_VAR = "FLOWMILL_EXPORT_BPF_SRC_FILE";

static constexpr auto FLOWMILL_DISABLE_HTTP_METRICS_VAR = "FLOWMILL_DISABLE_HTTP_METRICS";

static constexpr auto FLOWMILL_LABEL_CLUSTER_DEPRECATED_VAR = "FLOWMILL_AGENT_LABELS_ENVIRONMENT";
static constexpr auto FLOWMILL_LABEL_SERVICE_DEPRECATED_VAR = "FLOWMILL_AGENT_LABELS_SERVICE";
static constexpr auto FLOWMILL_LABEL_HOST_DEPRECATED_VAR = "FLOWMILL_AGENT_LABELS_HOST";
static constexpr auto FLOWMILL_LABEL_ZONE_DEPRECATED_VAR = "FLOWMILL_AGENT_LABELS_ZONE";

static constexpr auto FLOWMILL_NAMESPACE_OVERRIDE_VAR = "FLOWMILL_AGENT_NAMESPACE";
static constexpr auto FLOWMILL_CLUSTER_OVERRIDE_VAR = "FLOWMILL_AGENT_CLUSTER";
static constexpr auto FLOWMILL_SERVICE_OVERRIDE_VAR = "FLOWMILL_AGENT_SERVICE";
static constexpr auto FLOWMILL_HOST_OVERRIDE_VAR = "FLOWMILL_AGENT_HOST";
static constexpr auto FLOWMILL_ZONE_OVERRIDE_VAR = "FLOWMILL_AGENT_ZONE";

static void refill_log_rate_limit_cb(uv_timer_t *timer)
{
  LOG::refill_rate_limit_budget(200);
}

////////////////////////////////////////////////////////////////////////////////
void mount_debugfs_if_required()
{
  struct stat stat_buf = {0};

  int ret = stat("/sys/kernel/debug/tracing", &stat_buf);
  if (ret != 0) {
    if (errno == ENOENT) {
      /* directory doesn't exist, try to mount */
      ret = mount("debugfs", "/sys/kernel/debug/", "debugfs", 0, "");
      if (ret != 0) {
        throw std::system_error(errno, std::generic_category(), "could not mount debugfs on /sys/kernel/debug");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Disables stdout and/or stderr output if requested by the user
 * @param disable_stdout: should we disable stdout
 * @param disable_stderr: should we disable stderr
 *
 * @return 0 on success, non-zero on error (returns the value of errno)
 */
int control_stdout_stderr(bool disable_stdout, bool disable_stderr)
{
  // if user didn't ask to disable, we're done
  if ((disable_stdout == false) && (disable_stderr == false))
    return 0;

  // open /dev/null
  int null_fd = open("/dev/null", O_WRONLY);
  if (null_fd == -1)
    return errno;

  if (disable_stdout) {
    int res = dup2(null_fd, STDOUT_FILENO);
    if (res == -1) {
      int errno_copy = errno; // in case close changes errno
      close(null_fd);
      return errno_copy;
    }
  }

  if (disable_stderr) {
    int res = dup2(null_fd, STDERR_FILENO);
    if (res == -1) {
      int errno_copy = errno; // in case close changes errno
      close(null_fd);
      return errno_copy;
    }
  }

  // free the file descriptor we dup'd
  int res = close(null_fd);
  if (res == -1)
    return errno;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void set_resource_limits()
{
  struct rlimit l = {};
  l.rlim_max = AGENT_MAX_MEMORY;
  l.rlim_cur = l.rlim_max;
  int res = setrlimit(RLIMIT_DATA, &l);
  if (res != 0)
    LOG::warn("Could not set agent memory limit");

  res = nice(AGENT_NICE_VALUE);
  if (res == -1)
    LOG::warn("Could not set agent nice(2) priority");
}

////////////////////////////////////////////////////////////////////////////////
void get_uname(struct utsname &unamebuf)
{
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
}

void verify_kernel_blacklist(bool override, struct utsname &unamebuf)
{
  bool kernel_fail = false;
  struct blacklist_match_line {
    const char *sysname;
    const char *nodename;
    const char *release;
    const char *version;
    const char *machine;
  };
  blacklist_match_line failed_line;
  static blacklist_match_line kernel_blacklist[] = {
#include "kernel_blacklist.h"
  };
  int pat_num = 0;
  for (auto pat : kernel_blacklist) {
    bool match = true;
    if (match && pat.sysname && !std::regex_match(unamebuf.sysname, std::regex(pat.sysname))) {
      match = false;
    }
    if (match && pat.nodename && !std::regex_match(unamebuf.nodename, std::regex(pat.nodename))) {
      match = false;
    }
    if (match && pat.release && !std::regex_match(unamebuf.release, std::regex(pat.release))) {
      match = false;
    }
    if (match && pat.version && !std::regex_match(unamebuf.version, std::regex(pat.version))) {
      match = false;
    }
    if (match && pat.machine && !std::regex_match(unamebuf.machine, std::regex(pat.machine))) {
      match = false;
    }
    if (match) {
      kernel_fail = true;
      failed_line = pat;
      break;
    }
    pat_num++;
  }
  if (kernel_fail) {
    std::string failstr = fmt::format(
        "kernel blacklist check: pattern #{}:"
        "   sysname: {}\n"
        "  nodename: {}\n"
        "   release: {}\n"
        "   version: {}\n"
        "   machine: {}",
        pat_num,
        failed_line.sysname ? failed_line.sysname : "NULL",
        failed_line.nodename ? failed_line.nodename : "NULL",
        failed_line.release ? failed_line.release : "NULL",
        failed_line.version ? failed_line.version : "NULL",
        failed_line.machine ? failed_line.machine : "NULL");

    if (!override) {
      LOG::critical("Failed {}", failstr);
      exit(-1);
    } else {
      LOG::info("Overriding {}", failstr);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  /* Initialize libuv loop */
  uv_loop_t loop;
  int res = uv_loop_init(&loop);
  if (res != 0) {
    throw std::runtime_error("uv_loop_init failed");
  }

  // args parsing

  cli::ArgsParser parser("Flowmill agent.");
  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlag<u64> filter_ns(*parser, "nanoseconds", "Gap between subsequent reports", {"filter-ns"}, 10 * 1000 * 1000ull);
  args::ValueFlag<u64> socket_stats_interval_sec(
      *parser, "seconds", "Interval between sending socket stats", {"socket-stats-interval-sec"}, 10);

  args::ValueFlag<u64> metadata_timeout_us(
      *parser,
      "microseconds",
      "Microseconds to wait for cloud instance metadata",
      {"cloud-metadata-timeout-ms"},
      std::chrono::microseconds(1s).count());
  args::ValueFlag<std::string> conf_file(*parser, "config_file", "The location of the custom config file", {"config-file"}, "");
  args::Flag no_stdout(*parser, "no_stdout", "Disable stdout output", {"no-stdout"});
  args::Flag no_stderr(*parser, "no_stderr", "Disable stderr output", {"no-stderr"});
  args::Flag override_kernel_blacklist(
      *parser,
      "override_kernel_blacklist",
      "Override kernel blacklist (use at your own risk, can result in kernel panic)",
      {"override-kernel-blacklist"});
  auto host_distro = parser.add_arg<LinuxDistro>("host-distro", "Reports the linux distro that was detected on the host");
  auto kernel_headers_source =
      parser.add_arg<KernelHeadersSource>("kernel-headers-source", "Reports the method used to source kernel headers");
  auto entrypoint_error = parser.add_arg<EntrypointError>(
      "entrypoint-error", "Reports errors that took place before the agent was run", nullptr, EntrypointError::none);

  /* crash reporting */
  args::Flag disable_log_rate_limit(
      *parser, "disable_log_rate_limit", "Disable rate limit the logging", {"disable-log-rate-limit"});
  args::ValueFlag<std::string> docker_ns_label(
      *parser, "label", "Docker label to be used as namespace", {"docker-ns-label"}, "");

  auto disable_http_metrics = parser.add_env_flag(
      "disable-http-metrics", "Disable collection of HTTP metrics", FLOWMILL_DISABLE_HTTP_METRICS_VAR, false);

  // keeping "enable-http-metrics" around while we phase it out,
  // so that older deployments won't break
  // we're transitioning to opt-out (preferably always on)
  args::Flag enable_http_metrics_deprecated(
      *parser, "http_metrics", "Enable collection of HTTP metrics (deprecated)", {"enable-http-metrics"});

  args::Flag enable_userland_tcp_flag(
      *parser, "userland_tcp", "Enable userland tcp processing (experimental)", {"enable-userland-tcp"});

  auto force_docker_metadata = parser.add_flag("force-docker-metadata", "Forces the use of docker metadata");
  auto dump_docker_metadata = parser.add_flag("dump-docker-metadata", "Dump docker metadata for debug purposes");
  auto disable_nomad_metadata = parser.add_flag("disable-nomad-metadata", "Disables detection and use of Nomad metadata");

  auto disable_intake_tls = parser.add_flag("disable-tls", "Disable TLS when connecting to intake");

  args::ValueFlag<std::string> bpf_dump_file(
      *parser, "bpf-dump-file", "If set, dumps the stream of eBPF messages to the file given by this flag", {"bpf-dump-file"});

  auto report_bpf_debug_events =
      parser.add_flag("report-bpf-debug-events", "Whether bpf debug events should be reported to userland or not");

#ifdef CONFIGURABLE_BPF
  args::ValueFlag<std::string> bpf_file(*parser, "bpf_file", "File containing bpf code", {"bpf"}, "");
#endif // CONFIGURABLE_BPF

#ifndef NDEBUG
  auto schedule_bpf_lost_samples = parser.add_arg<std::chrono::seconds::rep>(
      "schedule-bpf-lost-samples",
      "internal development - will continuously, at the interval in seconds provided, simulate lost BPF samples (PERF_RECORD_LOST) in BufferedPoller to test KernelCollector restarts via that code path");
#endif

  parser.new_handler<LogWhitelistHandler<AgentLogKind>>("agent-log");
  parser.new_handler<LogWhitelistHandler<channel::Component>>("channel");
  parser.new_handler<LogWhitelistHandler<CloudPlatform>>("cloud-platform");
  parser.new_handler<LogWhitelistHandler<Utility>>("utility");

  auto &intake_config_handler = parser.new_handler<config::IntakeConfig::ArgsHandler>();

  SignalManager &signal_manager = parser.new_handler<SignalManager>(loop, "kernel-collector");

  if (auto result = parser.process(argc, argv); !result.has_value()) {
    return result.error();
  }

  /*
   * Set docker nameservice label from commandline flags if provided;
   * fallback to $DOCKER_NS_LABEL environment variable if that exists.
   */
  if (docker_ns_label.Matched()) {
    CgroupHandler::docker_ns_label_field = docker_ns_label.Get();
  } else {
    char const *docker_ns_label_env = std::getenv("DOCKER_NS_LABEL");
    if (docker_ns_label_env != nullptr) {
      CgroupHandler::docker_ns_label_field = std::string(docker_ns_label_env);
    }
  }

  /* set agent resource limits */
#if USE_ADDRESS_SANITIZER == 0
  set_resource_limits();
#endif

  /* disable stdout/stderr if requested */
  {
    int res = control_stdout_stderr(no_stdout, no_stderr);
    if (res != 0) {
      LOG::critical("Could not squelch stdout={} stderr={}: error={}", no_stdout, no_stderr, res);
      sleep(5); // make sure we don't spam too much
      exit(-1);
    }
  }

  auto agent_id = gen_agent_id();

  /* log agent version */
  LOG::info("Starting Flowmill Agent version {} ({})", versions::release, release_mode_string);
  LOG::info("Kernel Collector agent ID is {}", agent_id);

  /* get kernel version */
  struct utsname unamebuf;
  get_uname(unamebuf);

  /* check kernel blacklist */
  verify_kernel_blacklist(override_kernel_blacklist.Matched(), unamebuf);

  /*
   *  Set http_metrics flag if environment variable specified as well
   */
  bool const enable_http_metrics = !disable_http_metrics;
  static constexpr char const *enabled_disabled[] = {"Disabled", "Enabled"};
  LOG::info("HTTP Metrics: {}", enabled_disabled[enable_http_metrics]);

  LOG::info("Socket stats interval in seconds: {}", socket_stats_interval_sec.Get());

  /* acknowledge userland tcp */
  bool const enable_userland_tcp = enable_userland_tcp_flag.Matched();
  LOG::info("Userland TCP: {}", enabled_disabled[enable_userland_tcp]);

  /* Initialize curl */
  curlpp::initialize();

  std::chrono::microseconds const metadata_timeout(metadata_timeout_us.Get());

  /* AWS metadata */
  LOG::trace_in(CloudPlatform::aws, "--- resolving AWS metadata ---");
  auto const aws_metadata = AwsMetadata::fetch(metadata_timeout);
  if (aws_metadata) {
    aws_metadata->print_instance_metadata();
    aws_metadata->print_interfaces();
  } else {
    LOG::debug("Unable to fetch AWS metadata: {}", aws_metadata.error());
  }

  /* GCP metadata */
  LOG::trace_in(CloudPlatform::gcp, "--- resolving GCP metadata ---");
  auto const gcp_metadata = GcpInstanceMetadata::fetch(metadata_timeout);
  if (gcp_metadata) {
    gcp_metadata->print();
  } else {
    LOG::debug("Unable to fetch GCP metadata: {}", gcp_metadata.error());
  }

  /* read label overrides from environment if present */
  auto override_agent_namespace = std::string{try_get_env_var(FLOWMILL_NAMESPACE_OVERRIDE_VAR)};
  auto override_agent_cluster =
      std::string{try_get_env_var(FLOWMILL_CLUSTER_OVERRIDE_VAR, try_get_env_var(FLOWMILL_LABEL_CLUSTER_DEPRECATED_VAR))};
  auto override_agent_zone =
      std::string{try_get_env_var(FLOWMILL_ZONE_OVERRIDE_VAR, try_get_env_var(FLOWMILL_LABEL_ZONE_DEPRECATED_VAR))};
  auto override_agent_service =
      std::string{try_get_env_var(FLOWMILL_SERVICE_OVERRIDE_VAR, try_get_env_var(FLOWMILL_LABEL_SERVICE_DEPRECATED_VAR))};
  auto override_agent_host =
      std::string{try_get_env_var(FLOWMILL_HOST_OVERRIDE_VAR, try_get_env_var(FLOWMILL_LABEL_HOST_DEPRECATED_VAR))};

  /* Nomad metadata */
  if (!disable_nomad_metadata) {
    LOG::trace("--- resolving Nomad metadata ---");
    if (NomadMetadata const nomad_metadata; nomad_metadata) {
      nomad_metadata.print();

      // respect explicit overrides from environment variables

      // TODO: what's the equivalent of a namespace in Nomad?

      if (override_agent_cluster.empty() && !nomad_metadata.ns().empty()) {
        override_agent_cluster = std::string(nomad_metadata.ns());
      }

      if (override_agent_service.empty() && !nomad_metadata.task_name().empty()) {
        override_agent_service = std::string(nomad_metadata.task_name());
      }
    } else {
      LOG::debug("Unable to fetch Nomad metadata - environment variables not found");
    }
  } else {
    LOG::trace("--- skipping Nomad metadata resolution ---");
  }

  // resolve hostname
  std::string const hostname = get_host_name(MAX_HOSTNAME_LENGTH).recover([&](auto &error) {
    LOG::error("Unable to retrieve host information from uname: {}", error);
    return aws_metadata->id().valid() ? std::string(aws_metadata->id().value()) : "(unknown)";
  });
  LOG::info("Kernel Collector version {} ({}) started on host {}", versions::release, release_mode_string, hostname);

  config::ConfigFile configuration_data(config::ConfigFile::YamlFormat(), conf_file.Get());

  /* use label overrides if present */
  if (!override_agent_namespace.empty()) {
    LOG::debug("overriding agent namespace with '{}'", override_agent_namespace);
    configuration_data.labels()["namespace"] = override_agent_namespace;
  }
  if (!override_agent_cluster.empty()) {
    LOG::debug("overriding agent cluter with '{}'", override_agent_cluster);
    configuration_data.labels()["cluster"] = override_agent_cluster;
  }
  if (!override_agent_zone.empty()) {
    LOG::debug("overriding agent zone with '{}'", override_agent_zone);
    configuration_data.labels()["zone"] = override_agent_zone;
  }
  if (!override_agent_service.empty()) {
    LOG::debug("overriding agent service with '{}'", override_agent_service);
    configuration_data.labels()["service"] = override_agent_service;
  }
  if (!override_agent_host.empty()) {
    LOG::debug("overriding agent host with '{}'", override_agent_host);
    configuration_data.labels()["host"] = override_agent_host;
  }

  HostInfo host_info{
      .os = OperatingSystem::Linux,
      .os_flavor = integer_value(*host_distro),
      .os_version = gcp_metadata ? gcp_metadata->image() : "unknown",
      .kernel_headers_source = *kernel_headers_source,
      .kernel_version = unamebuf.release,
      .hostname = hostname};

  try {
    /* mount debugfs if it is not mounted */
    mount_debugfs_if_required();

    /* Read our BPF program*/
    /* resolve includes */
    std::string bpf_src((char *)agent_bpf_c, agent_bpf_c_len);
#ifdef CONFIGURABLE_BPF
    if (bpf_file.Matched()) {
      bpf_src = *read_file_as_string(bpf_file.Get().c_str()).try_raise();
    }
#endif

    u64 boot_time_adjustment = get_boot_time();
    /* insert time onto the bpf program */
    bpf_src = std::regex_replace(bpf_src, std::regex("BOOT_TIME_ADJUSTMENT"), fmt::format("{}uLL", boot_time_adjustment));
    bpf_src = std::regex_replace(bpf_src, std::regex("FILTER_NS"), fmt::format("{}", args::get(filter_ns)));
    bpf_src = std::regex_replace(bpf_src, std::regex("MAX_PID"), *read_file_as_string(MAX_PID_PROC_PATH).try_raise());
    bpf_src = std::regex_replace(
        bpf_src, std::regex("REPORT_DEBUG_EVENTS_PLACEHOLDER"), std::string(1, "01"[*report_bpf_debug_events]));

    if (std::string const out{try_get_env_var(FLOWMILL_EXPORT_BPF_SRC_FILE_VAR)}; !out.empty()) {
      if (auto const error = write_file(out.c_str(), bpf_src)) {
        LOG::error("ERROR: unable to write BPF source to '{}': {}", out, error);
      }
    }

    uv_timer_t refill_log_rate_limit_timer;
    if (!disable_log_rate_limit) {
      LOG::enable_rate_limit(5000);
      res = uv_timer_init(&loop, &refill_log_rate_limit_timer);
      if (res != 0) {
        throw std::runtime_error("Cannot init rate limit timer.");
      }

      res = uv_timer_start(
          &refill_log_rate_limit_timer,
          refill_log_rate_limit_cb,
          /*1s*/ 1000,
          /*1s*/ 1000);
      if (res != 0) {
        throw std::runtime_error("Cannot start rate limit timer.");
      }
    }

    /* initialize curl engine */
    auto curl_engine = CurlEngine::create(&loop);

    config::IntakeConfig intake_config(intake_config_handler.read_config());

    // Initialize our kernel telemetry collector
    KernelCollector kernel_collector{
        bpf_src,
        intake_config,
        boot_time_adjustment,
        aws_metadata.try_value(),
        gcp_metadata.try_value(),
        configuration_data.labels(),
        loop,
        *curl_engine,
        enable_http_metrics,
        enable_userland_tcp,
        socket_stats_interval_sec.Get(),
        CgroupHandler::CgroupSettings{
            .force_docker_metadata = *force_docker_metadata,
            .dump_docker_metadata = *dump_docker_metadata,
        },
        bpf_dump_file.Get(),
        host_info,
        *entrypoint_error};

    signal_manager.handle_signals({SIGINT, SIGTERM}, std::bind(&KernelCollector::on_close, &kernel_collector));

#ifndef NDEBUG
    std::unique_ptr<scheduling::Timer> debug_bpf_lost_samples_timer;
    if (schedule_bpf_lost_samples.given() && *schedule_bpf_lost_samples > 0) {
      std::chrono::seconds const timeout(*schedule_bpf_lost_samples);

      auto schedule_bpf_lost_samples = [&debug_bpf_lost_samples_timer, timeout]() {
        if (auto const result = debug_bpf_lost_samples_timer->defer(timeout)) {
          LOG::info("successfully scheduled inject_bpf_lost_samples() {} from now", timeout);
        } else {
          LOG::error("failed to schedule inject_bpf_lost_samples() {} from now: {}", timeout, result.error());
        }
      };

      auto inject_bpf_lost_samples = [&kernel_collector, schedule_bpf_lost_samples]() {
        kernel_collector.debug_bpf_lost_samples();
        schedule_bpf_lost_samples(); // reschedule another callback to inject bpf lost samples
      };

      debug_bpf_lost_samples_timer = std::make_unique<scheduling::Timer>(loop, inject_bpf_lost_samples);
      schedule_bpf_lost_samples(); // schedule the first callback to inject bpf lost samples
    }
#endif

    LOG::debug("starting event loop...");
    uv_run(&loop, UV_RUN_DEFAULT);
  } catch (std::system_error &e) {
    if (e.code().value() == EPERM) {
      print_troubleshooting_message_and_exit(host_info, TroubleshootItem::operation_not_permitted, e);
      return 1;
    }
    print_troubleshooting_message_and_exit(host_info, TroubleshootItem::unexpected_exception, e);
    return 1;
  } catch (std::exception &e) {
    print_troubleshooting_message_and_exit(host_info, TroubleshootItem::unexpected_exception, e);
    return 1;
  }
}
