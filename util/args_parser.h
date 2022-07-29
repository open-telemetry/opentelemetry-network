/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * A class to handle command line parameters.
 */

#include <util/debug.h>
#include <util/expected.h>

#include <args.hxx>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace cli {

class ArgsParser {
public:
  /**
   * Bitmasks for flags that can be automatically handled by this arguments parser.
   *
   * NOTE: use unique powers of two for enum values.
   */
  enum Flags {
    none = 0,
    /**
     * Flags for handling minimum level for logging.
     */
    log_levels = 1
  };

  static constexpr Flags default_flags = Flags::log_levels;

  /**
   * Constructs an args parser with given header and footer.
   *
   * flags: a bitwise or'ed set of flags to be automatically handled by the parser.
   */
  ArgsParser(std::string header, std::string footer, Flags flags = default_flags);

  /**
   * Constructs an args parser with given header.
   *
   * flags: a bitwise or'ed set of flags to be automatically handled by the parser.
   */
  ArgsParser(std::string header, Flags flags = default_flags);

  class FlagProxy {
  public:
    FlagProxy(args::ArgumentParser &parser, std::string const &name, std::string const &description, bool default_value);

    bool given() const;

    explicit operator bool() const;
    bool operator!() const { return !static_cast<bool>(*this); }
    bool operator*() const { return static_cast<bool>(*this); }

  private:
    args::Flag flag_;
    bool default_;
  };

  /**
   * Adds a boolean flag.
   */
  FlagProxy add_flag(std::string const &name, std::string const &description, bool default_value = false);

  /**
   * Adds a boolean flag that defaults to a given environment variable's value, if present.
   *
   * Order of priority is:
   *  1. command-line argument
   *  2. environment variable's value
   *  3. default value
   */
  FlagProxy
  add_env_flag(std::string const &name, std::string const &description, char const *env_var, bool default_value = false);

  template <typename T> class ArgProxy {
    static constexpr bool needs_proxy = std::is_enum_v<T>;
    using proxy_ref = std::conditional_t<needs_proxy, T, T &>;

  public:
    ArgProxy(args::ArgumentParser &parser, std::string const &name, std::string const &description, T const &default_value);

    bool given() const;

    explicit operator bool() const { return given(); }

    proxy_ref operator*();

    template <typename = std::enable_if_t<needs_proxy>> T *operator->();

  private:
    using arg_type = std::conditional_t<std::is_enum_v<T>, std::string, T>;
    args::ValueFlag<arg_type> arg_;
    bool default_;
  };

  /**
   * Adds a new command line flag of type `T` with given argument name,
   * description and default value.
   *
   * If null-terminated string `env_var` is not null and an environment
   * variable with that name exists, its value overrides `default_value`.
   *
   * If the value of the environment variable can't be converted to `T`,
   * `std::invalid_argument` is thrown.
   */
  template <typename T>
  ArgProxy<T>
  add_arg(std::string const &name, std::string const &description, char const *env_var = nullptr, T default_value = {});

  /**
   * Parses command line arguments and handles pre-defined flags.
   *
   * On error, returns an exit code.
   */
  Expected<bool, int> process(int argc, char **argv);

  /**
   * Compatibility with args::ArgumentsParser.
   */
  args::ArgumentParser *operator->() { return &parser_; }
  args::ArgumentParser &operator*() { return parser_; }

  class Handler {
  public:
    virtual ~Handler() {}

    virtual void handle() {}
  };

  /**
   * Add an args parser handler
   */
  template <class HandlerType, typename... Args> HandlerType &new_handler(Args &&... args);

  /**
   * Utility function for splitting argument lists
   */
  static void split_arguments(const std::string &argliststr, std::list<std::string> &argument_list);

private:
  args::ArgumentParser parser_;
  std::vector<std::unique_ptr<Handler>> handlers_;
};

} // namespace cli

#include <util/args_parser.inl>
