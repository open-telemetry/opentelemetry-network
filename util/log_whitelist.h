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

#pragma once

#include <util/args_parser.h>
#include <util/enum.h>

#include <spdlog/fmt/fmt.h>

#include <initializer_list>
#include <list>
#include <tuple>

/**
 * Implements a log whitelist.
 *
 * All entries are whitelisted by default.
 *
 * Separate whitelists are maintained for each `Which` enum class given.
 *
 * To keep things simple and the implementation small, bitfields are used.
 * Therefore `Which` enum class must have an arithmetic underlying type
 * convertible to `std::uint8_t` assuming integer values smaller than 64. This
 * limitation may be lifted in the future but this class is guaranteed to be
 * backwards compatible.
 */

// these function are not thread-safe for performance reasons
// call them only in main before any threads are created
template <typename Which> void log_whitelist_all();
template <typename Which> void log_whitelist_clear();
template <typename Which> void set_log_whitelist(std::list<Which> whitelist);
template <typename Which> void set_log_whitelist(std::initializer_list<Which> whitelist);
void log_whitelist_all_globally();

// this function is thread-safe
template <typename Whitelist, typename = std::enable_if_t<std::is_enum_v<Whitelist>>> bool is_log_whitelisted(Whitelist which);

// this function is thread-safe
template <typename... Whitelist> bool is_log_whitelisted(std::tuple<Whitelist...> const &which);

// args parser handler for whitelists
template <typename Which> class LogWhitelistHandler : public cli::ArgsParser::Handler {
public:
  LogWhitelistHandler(cli::ArgsParser &parser, std::string const &option_name);

  void handle() override;

private:
  std::string option_name_;
  cli::ArgsParser::ArgProxy<std::string> log_whitelist_;
};

#include <util/log_whitelist.inl>
