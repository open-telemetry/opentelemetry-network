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

#include <util/args_parser.h>

#include <util/environment_variables.h>
#include <util/log.h>
#include <util/log_whitelist.h>

#include <spdlog/spdlog.h>

#include <iostream>

namespace cli {

class LogLevelsHandler: public ArgsParser::Handler {
public:
  LogLevelsHandler(cli::ArgsParser &parser):
    no_log_file_(parser.add_flag("no-log-file", "Do not log to a log file.")),
    log_console_(parser.add_flag("log-console", "Log to console.")),
    trace_(parser.add_flag("trace", "Sets minimum log level to `trace` (very verbose)")),
    debug_(parser.add_flag("debug", "Sets minimum log level to `debug` (very verbose)")),
    info_(parser.add_flag("info", "Sets minimum log level to `info`")),
    warn_(parser.add_flag("warning", "Sets minimum log level to `warning`")),
    error_(parser.add_flag("error", "Sets minimum log level to `error`")),
    crit_(parser.add_flag("critical", "Sets minimum log level to `critical`")),
    whitelist_all_(parser.add_flag("log-whitelist-all", "enable all whitelists"))
  {}

  void handle() override {
    auto const log_file = std::string(LOG::log_file_path());
    LOG::init(*log_console_, no_log_file_ ? nullptr : &log_file);

    if (trace_) {
      if constexpr (!DEBUG_LOG) {
        std::cout << "Warning: `trace` log level requested but can't be enabled on a release build"
          << std::endl;
      }
      spdlog::set_level(spdlog::level::trace);
    } else if (debug_) {
      if constexpr (!DEBUG_LOG) {
        std::cout << "Warning: `debug` log level requested but can't be enabled on a release build"
          << std::endl;
      }
      spdlog::set_level(spdlog::level::debug);
    } else if (info_) {
      spdlog::set_level(spdlog::level::info);
    } else if (warn_) {
      spdlog::set_level(spdlog::level::warn);
    } else if (error_) {
      spdlog::set_level(spdlog::level::err);
    } else if (crit_) {
      spdlog::set_level(spdlog::level::critical);
    }

    if (whitelist_all_) {
      log_whitelist_all_globally();
    }
  }

private:
  cli::ArgsParser::FlagProxy no_log_file_;
  cli::ArgsParser::FlagProxy log_console_;
  cli::ArgsParser::FlagProxy trace_;
  cli::ArgsParser::FlagProxy debug_;
  cli::ArgsParser::FlagProxy info_;
  cli::ArgsParser::FlagProxy warn_;
  cli::ArgsParser::FlagProxy error_;
  cli::ArgsParser::FlagProxy crit_;
  cli::ArgsParser::FlagProxy whitelist_all_;
};

ArgsParser::ArgsParser(std::string header, std::string footer, Flags flags):
  parser_(std::move(header), std::move(footer))
{
  if (flags & Flags::log_levels) {
    handlers_.emplace_back(std::make_unique<LogLevelsHandler>(*this));
  }
}

ArgsParser::ArgsParser(std::string header, Flags flags):
  ArgsParser(std::move(header), "", flags)
{}

ArgsParser::FlagProxy::FlagProxy(
  args::ArgumentParser &parser,
  std::string const &name,
  std::string const &description,
  bool default_value
):
  flag_(parser, name, description, {name}),
  default_(default_value)
{}

bool ArgsParser::FlagProxy::given() const {
  return flag_.Matched();
}

ArgsParser::FlagProxy::operator bool() const {
  return given()
    ? flag_
    : default_;
}

ArgsParser::FlagProxy ArgsParser::add_flag(
  std::string const &name,
  std::string const &description,
  bool default_value
) {
  return FlagProxy(parser_, name, description, default_value);
}

ArgsParser::FlagProxy ArgsParser::add_env_flag(
  std::string const &name,
  std::string const &description,
  char const *env_var,
  bool default_value
) {
  assert(env_var);

  if (auto const value = try_get_env_var(env_var); !value.empty()) {
    default_value = value == "true" || value == "True" || value == "TRUE"
      || value == "yes" || value == "Yes" || value == "YES"
      || value == "1";
  }

  return add_flag(name, description, default_value);
}

Expected<bool, int> ArgsParser::process(int argc, char **argv) {
  try {
    parser_.ParseCLI(argc, argv);
  } catch (args::Help const &) {
    std::cout << parser_.Help();
    return {unexpected, 0};
  } catch (args::Error const &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser_.Help();
    return {unexpected, -9};
  }

  for (auto &handler: handlers_) {
    handler->handle();
  }

  return true;
}

void ArgsParser::split_arguments(const std::string &argliststr, std::list<std::string> & argument_list)
{
  // Tokenize string
  const char *delims=", |;\t\r\n";
  std::size_t start = argliststr.find_first_not_of(delims), end = 0;
  while((end = argliststr.find_first_of(delims, start)) != std::string::npos)
  {
      argument_list.push_back(argliststr.substr(start, end - start));
      start = argliststr.find_first_not_of(delims, end);
  }
  if(start != std::string::npos) {
    argument_list.push_back(argliststr.substr(start));
  }
}

} // namespace cli
