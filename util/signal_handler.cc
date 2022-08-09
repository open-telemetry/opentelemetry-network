// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

/**
 * NOTE on ordering: if possible, put initialization after Google Breakpad
 *   has been configured, so breakpad can catch exceptions in your code.
 *
 * If that's not possible / desirable, make sure the code doesn't fail
 *   when the `minidump-path` parameter is active: when sending a minidump,
 *   there parameters passed only touch on minidump sending. Trying to
 *   initialize other systems in this case is likely to fail and interrupt
 *   the minidump flow -- which we should avoid.
 */

#include <util/signal_handler.h>

#include <common/constants.h>

#include <otlp/otlp_emitter.h>
#include <otlp/otlp_grpc_metrics_client.h>
#include <otlp/otlp_request_builder.h>

#include <util/aws_instance_metadata.h>
#include <util/environment_variables.h>
#include <util/file_ops.h>
#include <util/gcp_instance_metadata.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/system_ops.h>
#include <util/time.h>

#include <sys/utsname.h>
#include <sys/wait.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#define COLLECT_MINIDUMP_FLAG "collect-minidump"

// The maximum number of files allowed in the minidump directory.
static constexpr int MAX_MINIDUMP_DIR_SIZE_FILES = 3;

//  The maximum aggregate size in bytes of files in the minidump directory.
static constexpr int MAX_MINIDUMP_DIR_SIZE_BYTES = 15 << 20; // 15 MiB

// Maximum number of crash report directories to leave in the minidump directory.
static constexpr int MAX_CRASH_REPORTS = 3;
// Maximum allowed aggregate size (in bytes) of files in crash report directories.
static constexpr int MAX_CRASH_REPORTS_SIZE = 30 << 20; // 30 MiB

// Directory name suffix that all crash report directories will have.
static constexpr std::string_view CRASH_REPORT_DIR_SUFFIX = ".crash";
// Name of the file in which the configuration parameters will be stored.
static constexpr std::string_view PARAMETERS_FILE_NAME = "parameters.txt";

// Directory to which minidump files will be written by breakpad upon crash
static constexpr auto MINIDUMP_DIR_VAR = "EBPF_NET_MINIDUMP_DIR";
static constexpr std::string_view MINIDUMP_DIR_DEFAULT = "/tmp/ebpf_net_minidump";

static constexpr auto DEBUG_MODULE_NAME_VAR = "EBPF_NET_DEBUG_MODULE_NAME";
static constexpr auto DEBUG_MODULE_ID_VAR = "EBPF_NET_DEBUG_MODULE_ID";

static constexpr auto CLUSTER_NAME_VAR = "EBPF_NET_CLUSTER_NAME";

// NB: not defining the crash metric host will disable the crash metric feature
static constexpr auto CRASH_METRIC_HOST_VAR = "EBPF_NET_CRASH_METRIC_HOST";
static constexpr auto CRASH_METRIC_PORT_VAR = "EBPF_NET_CRASH_METRIC_PORT";

static constexpr std::string_view CRASH_METRIC_NAME = "ebpf_net.unplanned_exit";

static constexpr std::chrono::microseconds METADATA_TIMEOUT = 1s;

/**
 * Callback when a crashdump happens via Google Breakpad
 */
static bool breakpad_callback(const google_breakpad::MinidumpDescriptor &descriptor, void *context, bool succeeded)
{
  printf(
      "--------------------------------------------------------------------------------------------\n"
      "CRASH DETECTED - collecting crash minidump in `%s`...\n",
      descriptor.path());

  /* get the parent's pid */
  auto const parent_pid = getpid();

  /* get the path to /proc/<pid>/exe */
  char proc_exe_file[PATH_MAX + 1];
  if (auto const error = snprintf(proc_exe_file, PATH_MAX + 1, "/proc/%d/exe", parent_pid);
      error < 0 || error >= PATH_MAX + 1) {
    puts("ERROR: could not result executable path to get a crash dump.");
    return succeeded;
  }

  /* get the binary path */
  char current_binary_path[PATH_MAX + 1];
  auto const len = ::readlink(proc_exe_file, current_binary_path, PATH_MAX);
  if (len < 0 || len >= PATH_MAX + 1) {
    puts("ERROR: could not resolve executable filename to get a crash dump.");
    return succeeded;
  }
  current_binary_path[len] = '\0';

  /* fork to send the minidump
   *
   * NOTE: we're using sys_fork instead of glibc's version, because glibc's
   *       wrapper is locking the malloc arena before doing the actual fork.
   *       Besides being unnecessary in this case -- the program is bombing
   *       out anyway -- it can actually cause the program to hang if the
   *       breakpad is triggered by a memory allocation related bug, e.g. a
   *       double-free.
   */
  auto pid = sys_fork();
  if (pid == -1) {
    puts("ERROR: could not fork to send error report");
  } else if (!pid) {
    /* child */
    printf("executing crash minidump handler `%s --" COLLECT_MINIDUMP_FLAG " %s`...\n", current_binary_path, descriptor.path());

    auto const exec_result = ::execl(
        current_binary_path,
        current_binary_path,
        /* TODO: pass in log flags properly */ "--log-console",
        "--no-log-file",
        "--" COLLECT_MINIDUMP_FLAG,
        descriptor.path(),
        (char *)NULL);

    if (exec_result == -1) {
      /* ERROR */
      printf("ERROR: child could not exec, errno %d: %s\n", errno, ::strerror(errno));
      exit(-1);
    }
  } else {
    printf("waiting for crash minidump handler process (pid=%d)\n", pid);
    int status = 0;
    ::waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      printf("crash minidump handler process terminated normally with exit code %d\n", WEXITSTATUS(status));
    } else {
      puts("crash minidump handler process terminated abnormally\n");
    }
    /* exit with an error -- we had a crash. */
    exit(-1);
  }

  /* parent, return */
  return succeeded;
}

SignalManager::SignalManager(cli::ArgsParser &parser, ::uv_loop_t &loop, std::string_view product)
    : loop_(loop),
      product_(product),
      module_name_(try_get_env_var(DEBUG_MODULE_NAME_VAR, product_)),
      module_id_(try_get_env_var(DEBUG_MODULE_ID_VAR)),
      disable_crash_report_(parser.add_flag("disable-crash-report", "disables minidump / crash reporter")),
      minidump_dir_{try_get_env_var(MINIDUMP_DIR_VAR, MINIDUMP_DIR_DEFAULT)},
      minidump_path_(parser.add_arg<std::string>(COLLECT_MINIDUMP_FLAG, "internal crash reporting")),
      breakpad_descriptor_(minidump_dir_)

#ifndef NDEBUG
      ,
      crash_(parser.add_flag("crash", "internal development - force a SIGSEGV")),
      schedule_crash_(parser.add_arg<std::chrono::seconds::rep>(
          "schedule-crash", "internal development - will force a SIGSEGV after given number of seconds"))
#endif
{}

#ifndef NDEBUG
static void cause_crash()
{
  LOG::critical("simulating crash...");
  volatile auto *a = (char *)NULL;
  *a = 1;
}
#endif

void emit_crash_metric(std::map<std::string, std::string> parameters, std::string_view host, std::string_view port)
{
  if (host.empty()) {
    std::cerr << "No host specified to emit a crash metric. Skipping sending a crash metric." << std::endl;
    return;
  }

  auto now = std::chrono::system_clock::now();
  auto timestamp_ns = std::chrono::nanoseconds(now.time_since_epoch());
  std::string uri = std::string(host) + ":" + std::string(port);

  otlp_client::OtlpEmitter emitter(uri);
  emitter(otlp_client::OtlpRequestBuilder().metric(CRASH_METRIC_NAME).sum().number_data_point(1u, parameters, timestamp_ns));
}

void SignalManager::handle()
{
  if (!minidump_path_.given()) {
    LOG::debug("setting up breakpad...");
    setup_breakpad();
  } else {
    LOG::debug("handling crash minidump...");
    handle_minidump();
  }

#ifndef NDEBUG
  if (schedule_crash_.given()) {
    std::chrono::seconds const timeout{*schedule_crash_ < 0 ? 0 : *schedule_crash_};
    LOG::warn("scheduling a crash {} from now, as requested", timeout);
    crash_timer_ = std::make_unique<scheduling::Timer>(loop_, &cause_crash);
    LOG::debug("created crash scheduler object");
    if (auto const result = crash_timer_->defer(timeout)) {
      LOG::warn("successfully scheduled a crash after {} from now", timeout);
    } else {
      LOG::error("failed to schedule crash after {} from now: {}", timeout, result.error());
    }
  }
#endif
}

void SignalManager::setup_breakpad()
{
  create_directory(minidump_dir_.c_str());

  if (disable_crash_report_) {
    LOG::info("skipping minidump / crash reporter setup");
  } else {
    LOG::debug("setting up breakpad...");
    breakpad_exception_handler_.emplace(breakpad_descriptor_, nullptr, breakpad_callback, nullptr, true, -1);
  }

  // avoid generating core dumps
  struct rlimit const core_dump_limit = {.rlim_cur = 0, .rlim_max = 0};
  ::setrlimit(RLIMIT_CORE, &core_dump_limit);

#ifndef NDEBUG
  if (*crash_) {
    LOG::warn("forcing a crash now, as requested");
    cause_crash();
  }
#endif

  /* ignore SIGPIPE: http://docs.libuv.org/en/v1.x/guide/filesystem.html?highlight=sigpipe */
  ::signal(SIGPIPE, SIG_IGN);
}

void SignalManager::handle_minidump()
{
  auto const report_dir_name =
      string_time(std::chrono::system_clock::now(), "%Y-%m-%d_%H-%M-%S") + std::string(CRASH_REPORT_DIR_SUFFIX);
  auto const report_dir_path = std::filesystem::path(minidump_dir_) / report_dir_name;

  // Create a new directory for this crash report.
  std::error_code ec;
  if (!std::filesystem::create_directory(report_dir_path, ec)) {
    std::cerr << "creating directory " << report_dir_path << " failed with error " << ec << std::endl;
    // We failed to create a crash report. Before we bomb out we have to make
    // sure we're not filling the minidump directory with files.
    cleanup_directory(minidump_dir_.c_str(), MAX_MINIDUMP_DIR_SIZE_FILES, MAX_MINIDUMP_DIR_SIZE_BYTES);
    exit(1);
  }

  std::cout << "saving crash report to " << report_dir_path << std::endl;

  ////////////////
  // parameters //
  ////////////////

  std::map<std::string, std::string> parameters;

  parameters["cluster_name"] = std::string{try_get_env_var(CLUSTER_NAME_VAR)};

  parameters["product"] = product_;

  parameters["module_name"] = module_name_;
  parameters["module_id"] = module_id_;

  parameters["version_major"] = std::to_string(versions::release.major());
  parameters["version_minor"] = std::to_string(versions::release.minor());
  parameters["version_build"] = std::to_string(versions::release.build());
  parameters["version_signature"] = std::string(versions::release.signature());

  auto const aws_metadata = AwsMetadata::fetch(METADATA_TIMEOUT);
  if (aws_metadata) {
    parameters["aws_az"] = aws_metadata->az().value();
    parameters["aws_iam_role"] = aws_metadata->iam_role().value();
    parameters["aws_id"] = aws_metadata->id().value();
    parameters["aws_type"] = aws_metadata->type().value();
  }

  auto const gcp_metadata = GcpInstanceMetadata::fetch(METADATA_TIMEOUT);
  if (gcp_metadata) {
    parameters["gcp_hostname"] = gcp_metadata->hostname();
    parameters["gcp_name"] = gcp_metadata->name();
    parameters["gcp_id"] = gcp_metadata->id();
    parameters["gcp_az"] = gcp_metadata->az();
    parameters["gcp_type"] = gcp_metadata->type();
  }

  if (struct utsname buffer; !::uname(&buffer)) {
    parameters["uname_sysname"] = buffer.sysname;
    parameters["uname_nodename"] = buffer.nodename;
    parameters["uname_release"] = buffer.release;
    parameters["uname_version"] = buffer.version;
    parameters["uname_machine"] = buffer.machine;
  }

  parameters["host"] = get_host_name(MAX_HOSTNAME_LENGTH).recover([&](auto &error) {
    LOG::error("Unable to retrieve host information from uname: {}", error);
    return aws_metadata ? aws_metadata->id().valid() ? std::string(aws_metadata->id().value()) : "(unknown-aws)"
                        : gcp_metadata ? gcp_metadata->hostname() : "(unknown)";
  });

  // Save parameters to a file in the crash report directory.
  if (std::ofstream file(report_dir_path / PARAMETERS_FILE_NAME, std::ofstream::out); file.is_open()) {
    for (auto &[name, value] : parameters) {
      file << name << ": " << value << std::endl;
    }
  } else {
    std::cerr << "failed to create file " << report_dir_path / PARAMETERS_FILE_NAME << std::endl;
  }

  ///////////
  // files //
  ///////////

  // Move minidump file to the crash report directory.
  if (std::filesystem::path minidump_path = *minidump_path_; file_exists(minidump_path.c_str(), {FileAccess::read})) {
    std::filesystem::rename(minidump_path, report_dir_path / minidump_path.filename());
  }

  // Copy log file into the crash report directory.
  if (std::filesystem::path log_path = LOG::log_file_path(); file_exists(log_path.c_str(), {FileAccess::read})) {
    std::filesystem::copy(log_path, report_dir_path);
  }


  /////////////
  // cleanup //
  /////////////

  // Remove old crash report directories.
  // Since crash report directories are flat (they don't contain subdirs), we use zero for the max_depth parameter.
  cleanup_directory_subdirs(minidump_dir_.c_str(), MAX_CRASH_REPORTS, MAX_CRASH_REPORTS_SIZE, 0, CRASH_REPORT_DIR_SUFFIX);

  ////////////////
  // emit crash //
  ////////////////
  emit_crash_metric(parameters, try_get_env_var(CRASH_METRIC_HOST_VAR), try_get_env_var(CRASH_METRIC_PORT_VAR, "4317"));

  exit(0);
}

void SignalManager::handle_signals(std::initializer_list<int> signal_numbers, std::function<void()> on_signal)
{
  for (auto const signal_number : signal_numbers) {
    signals_.emplace_back(*this, on_signal, signal_number);
  }
}

void SignalManager::clear()
{
  signals_.clear();
}

////////////////////
// signal_handler //
////////////////////

void SignalManager::SignalHandler::signal_handler(uv_signal_t *handle, int signal_number)
{
  auto &handler = *reinterpret_cast<SignalManager::SignalHandler *>(handle->data);
  assert(signal_number == handler.signal_number());
  handler.on_signal();
}

SignalManager::SignalHandler::SignalHandler(SignalManager &manager, std::function<void()> on_signal, int signal_number)
    : manager_(manager), on_signal_(std::move(on_signal))
{
  if (auto const error = ::uv_signal_init(&manager_.loop(), &handler_)) {
    throw std::runtime_error(fmt::format("Could not init handler for signal {}", signal_number));
  }

  handler_.data = this;

  if (auto const error = ::uv_signal_start(&handler_, signal_handler, signal_number)) {
    throw std::runtime_error(fmt::format("Could not start handler for signal {}", signal_number));
  }
}

static void signal_handle_close_cb(uv_handle_t *handle)
{
  LOG::debug("Closed a signal handler handle");
}

SignalManager::SignalHandler::~SignalHandler()
{
  ::uv_signal_stop(&handler_);
  ::uv_close(reinterpret_cast<uv_handle_t *>(&handler_), signal_handle_close_cb);
}

void SignalManager::SignalHandler::on_signal()
{
  LOG::info("Caught signal {}", handler_.signum);

  // call on_signal callback
  if (on_signal_) {
    on_signal_();
  }

  manager_.clear();

  ::exit(-handler_.signum);
}
