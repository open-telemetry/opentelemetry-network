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
#include <config/intake_config.h>
#include <util/aws_instance_metadata.h>
#include <util/environment_variables.h>
#include <util/file_ops.h>
#include <util/gcp_instance_metadata.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/minidump_sender.h>
#include <util/system_ops.h>
#include <util/url.h>

#include <sys/utsname.h>
#include <sys/wait.h>

#include <chrono>
#include <sstream>

// Directory to which minidump files will be written by breakpad upon crash

// The maximum number of files allowed in the minidump directory.
static constexpr int MAX_MINIDUMP_DIR_SIZE_FILES = 3;

#define COLLECT_MINIDUMP_FLAG "collect-minidump"

//  The maximum aggregate size in bytes of files in the minidump directory.
static constexpr int MAX_MINIDUMP_DIR_SIZE_BYTES = 15 << 20; // 15 MiB

static constexpr auto FLOWMILL_CLUSTER_NAME_VAR = "FLOWMILL_CLUSTER_NAME";

static constexpr auto FLOWMILL_MINIDUMP_DIR_VAR = "FLOWMILL_MINIDUMP_DIR";
static constexpr std::string_view FLOWMILL_MINIDUMP_DIR = "/tmp/flowmill-minidump";

static constexpr auto FLOWMILL_DEBUG_MODULE_NAME_VAR = "FLOWMILL_DEBUG_MODULE_NAME";
static constexpr auto FLOWMILL_DEBUG_MODULE_ID_VAR = "FLOWMILL_DEBUG_MODULE_ID";

static constexpr auto FLOWMILL_CRASH_COLLECTOR_HOST_VAR = "FLOWMILL_CRASH_COLLECTOR_HOST";
static constexpr auto FLOWMILL_CRASH_COLLECTOR_HOST = "https://app.flowmill.com";
static constexpr auto FLOWMILL_CRASH_COLLECTOR_PATH_VAR = "FLOWMILL_CRASH_COLLECTOR_PATH";
static constexpr auto FLOWMILL_CRASH_COLLECTOR_PATH = "/api/v1/minidump/";

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
    printf(
        "executing crash minidump uploader `%s --" COLLECT_MINIDUMP_FLAG " %s`...\n", current_binary_path, descriptor.path());

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
    printf("waiting for crash minidump uploader process (pid=%d)\n", pid);
    int status = 0;
    ::waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      printf("crash minidump uploader process terminated normally with exit code %d\n", WEXITSTATUS(status));
    } else {
      puts("crash minidump uploader process terminated abnormally\n");
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
      module_name_(try_get_env_var(FLOWMILL_DEBUG_MODULE_NAME_VAR, product_)),
      module_id_(try_get_env_var(FLOWMILL_DEBUG_MODULE_ID_VAR)),
      disable_crash_report_(parser.add_flag("disable-crash-report", "disables minidump / crash reporter")),
      minidump_dir_{try_get_env_var(FLOWMILL_MINIDUMP_DIR_VAR, FLOWMILL_MINIDUMP_DIR)},
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

void SignalManager::handle()
{
  if (!minidump_path_.given()) {
    LOG::debug("setting up breakpad...");
    setup_breakpad();
  } else {
    LOG::debug("uploading crash minidump...");
    upload_minidump();
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

void SignalManager::upload_minidump()
{
  auto const minidump_url = format_url(
      std::string{try_get_env_var(FLOWMILL_CRASH_COLLECTOR_HOST_VAR, FLOWMILL_CRASH_COLLECTOR_HOST)},
      try_get_env_var(FLOWMILL_CRASH_COLLECTOR_PATH_VAR, FLOWMILL_CRASH_COLLECTOR_PATH));

  if (minidump_url.empty()) {
    LOG::error("no valid crash collection URL provided");
    return;
  }

  LOG::warn("uploading crash minidump to '{}'...", minidump_url);

  auto const intake_config = config::IntakeConfig::read_from_env();
  auto const proxy_config = intake_config.proxy();
  auto const proxy = [proxy_config]() -> std::string {
    if (!proxy_config) {
      return {};
    }

    std::string result = proxy_config->host();
    assert(!result.empty());

    if (!proxy_config->port().empty()) {
      result.push_back(':');
      result.append(proxy_config->port());
    }

    return result;
  }();

  if (!proxy.empty()) {
    LOG::info("using proxy server {}", proxy);
  }

  ////////////////
  // parameters //
  ////////////////

  std::map<std::string, std::string> parameters;

  parameters["cluster_name"] = std::string{try_get_env_var(FLOWMILL_CLUSTER_NAME_VAR)};

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

  parameters["intake_name"] = intake_config.name();
  parameters["intake_host"] = intake_config.host();
  parameters["intake_port"] = intake_config.port();

  parameters["proxy_host"] = proxy_config ? proxy_config->host() : "";
  parameters["proxy_port"] = proxy_config ? proxy_config->port() : "";

  ///////////
  // files //
  ///////////

  std::map<std::string, std::string> files;

  if (auto const path = *minidump_path_; file_exists(path.c_str(), {FileAccess::read})) {
    files["minidump"] = path;
  }

  if (auto const log_path = LOG::log_file_path(); file_exists(log_path.data(), {FileAccess::read})) {
    files["log_file"] = log_path;
  }

  ////////////
  // submit //
  ////////////

  bool success = false;
  if (MinidumpSender sender; sender.send(minidump_url, headers_, parameters, files, proxy)) {
    LOG::warn("successfully uploaded crash minidump to {}", minidump_url);
    success = true;
  } else {
    LOG::error("unable to upload crash minidump: ({}) {}", sender.response_code(), sender.error_description());
  }

  cleanup_directory(minidump_dir_.c_str(), MAX_MINIDUMP_DIR_SIZE_FILES, MAX_MINIDUMP_DIR_SIZE_BYTES);

  exit(!success);
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
