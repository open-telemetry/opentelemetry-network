// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <type_traits>
#include <utility>

#include <iostream>

#include <cassert>
#include <cstdint>

class LogWhitelistRegistry {
public:
  class Proxy {
  public:
    Proxy(void (*whitelist_all)());

    void (*whitelist_all)();
    Proxy *next = nullptr;
  };

  static void register_log_whitelist(Proxy &proxy);

  static void whitelist_all();

private:
  static Proxy *head_;
};

template <typename Which> class LogWhitelist {
  static_assert(std::is_enum_v<Which>, "Which must be an enum class");

public:
  // these functions are not thread-safe for performance reasons
  // call them only in main before any threads are created
  static void whitelist_all();
  static void set_whitelist(std::initializer_list<Which> whitelist = {});
  static void set_whitelist(std::list<Which> whitelist);

  static bool is_whitelisted(Which which);

  static LogWhitelistRegistry::Proxy proxy;

private:
  static std::uint64_t whitelist_;
  static constexpr auto const max_filter_ = 64;
};

template <typename Which> std::uint64_t LogWhitelist<Which>::whitelist_ = 0;

template <typename Which> LogWhitelistRegistry::Proxy LogWhitelist<Which>::proxy{whitelist_all};

template <typename Which> void LogWhitelist<Which>::whitelist_all()
{
  whitelist_ = static_cast<std::uint64_t>(~static_cast<std::uint64_t>(0));
}

template <typename Which> void LogWhitelist<Which>::set_whitelist(std::initializer_list<Which> whitelist)
{
  whitelist_ = 0;

  for (auto const which : whitelist) {
    auto const filter = static_cast<std::underlying_type_t<Which>>(which);
    assert(filter < max_filter_);
    whitelist_ |= static_cast<std::uint64_t>(static_cast<std::uint64_t>(1) << static_cast<std::uint64_t>(filter));
  }
}

template <typename Which> void LogWhitelist<Which>::set_whitelist(std::list<Which> whitelist)
{
  whitelist_ = 0;

  for (auto const which : whitelist) {
    auto const filter = static_cast<std::underlying_type_t<Which>>(which);
    assert(filter < max_filter_);
    whitelist_ |= static_cast<std::uint64_t>(static_cast<std::uint64_t>(1) << static_cast<std::uint64_t>(filter));
  }
}

template <typename Which> bool LogWhitelist<Which>::is_whitelisted(Which which)
{
  return (whitelist_ & static_cast<std::uint64_t>(static_cast<std::uint64_t>(1) << static_cast<std::uint64_t>(which))) != 0;
}

template <typename Which> void log_whitelist_all()
{
  LogWhitelist<Which>::whitelist_all();
}

template <typename Which> void log_whitelist_clear()
{
  LogWhitelist<Which>::set_whitelist({});
}

template <typename Which> void set_log_whitelist(std::initializer_list<Which> whitelist)
{
  LogWhitelist<Which>::set_whitelist(std::move(whitelist));
}

template <typename Which> void set_log_whitelist(std::list<Which> whitelist)
{
  LogWhitelist<Which>::set_whitelist(std::move(whitelist));
}

template <typename Whitelist, typename = std::enable_if_t<std::is_enum_v<Whitelist>>> bool is_log_whitelisted(Whitelist which)
{
  return LogWhitelist<Whitelist>::is_whitelisted(which);
}

template <typename... Whitelist, std::size_t... Index>
bool is_log_whitelisted_impl(std::tuple<Whitelist...> const &which, std::index_sequence<Index...>)
{
  return (true && ... && (is_log_whitelisted(std::get<Index>(which))));
}

template <typename... Whitelist> bool is_log_whitelisted(std::tuple<Whitelist...> const &which)
{
  return is_log_whitelisted_impl(which, std::make_index_sequence<sizeof...(Whitelist)>{});
}

// LogWhitelistHandler

template <typename Which>
LogWhitelistHandler<Which>::LogWhitelistHandler(cli::ArgsParser &parser, std::string const &option_name)
    : option_name_(option_name), log_whitelist_(parser.add_arg<std::string>(fmt::format("log-whitelist-{}", option_name), [&] {
        auto message = fmt::format("Comma separated list of logs to whitelist from {{", option_name);
        auto const &values = enum_traits<Which>::values;
        for (std::size_t i = 0; i < values.size(); ++i) {
          if (i) {
            message.append(", ");
          }
          message.append(to_string(values[i]));
        }
        message.push_back('}');
        return message;
      }()))
{
  LogWhitelistRegistry::register_log_whitelist(LogWhitelist<Which>::proxy);
}

template <typename Which> void LogWhitelistHandler<Which>::handle()
{
  std::list<std::string> to_parse;
  cli::ArgsParser::split_arguments(*log_whitelist_, to_parse);

  if (to_parse.size() == 1 && to_parse.front() == "*") {
    log_whitelist_all<Which>();
    return;
  }

  std::list<std::string> left_overs;

  std::list<Which> enum_list;
  for (auto const &token : to_parse) {
    Which v;
    if (enum_from_string(token, v)) {
      enum_list.push_back(v);
    } else {
      left_overs.push_back(token);
    }
  }

  if (enum_list.size() > 0) {
    set_log_whitelist<Which>(enum_list);
  }

  if (left_overs.size() > 0) {
    std::string err;
    for (auto const &s : left_overs) {
      if (err.size() > 0) {
        err += ",";
      }
      err += s;
    }
    throw std::runtime_error(fmt::format("Unable to enable requested logs for whitelist '{}': {}", option_name_, err));
  }
}
