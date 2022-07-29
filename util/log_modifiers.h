/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include <cassert>

namespace logger {

template <typename... Args>
constexpr inline bool is_logging_overhead =
    ((std::is_same_v<std::string, std::decay_t<Args>> && std::is_rvalue_reference_v<Args>) || ... || false);

template <typename... Args> static constexpr inline void check_logging_overhead()
{
  static_assert(
      !is_logging_overhead<Args...>,
      "\n"
      "  ====================\n"
      "  = LOGGING OVERHEAD =\n"
      "  ===================================================================================\n"
      "  Detected: temporary dynamic allocation of string for the sole purpose of logging.\n"
      "\n"
      "  Use `std::string_view` instead or see `util/log_modifiers.h` for low cost alternatives.\n"
      "\n"
      "  For protobuf messages, avoid `DebugString()`. Instead, include `util/protobuf_log.h`\n"
      "  and pass a message reference directly to the logger.\n"
      "  ===================================================================================\n");
}

} // namespace logger

////////////
// waived //
////////////

namespace logger::impl {

template <typename T> struct waived_t {
  constexpr explicit waived_t(T const &what) : what_(what) {}

  // make sure this is not used as a non-temporary
  waived_t(waived_t const &) = delete;
  waived_t(waived_t &&) = delete;
  waived_t &operator=(waived_t const &) = delete;
  waived_t &operator=(waived_t &&) = delete;

  template <typename Out> friend Out &&operator<<(Out &&out, waived_t const &what)
  {
    out << what.what_;
    return std::forward<Out>(out);
  }

private:
  T const &what_;
};

} // namespace logger::impl

/**
 * This is an unfortunate but necessary compatibility escape hatch for broken libraries
 * over which we have no control of, or special cases where temporaries are legitimate.
 *
 * Example:
 *
 *  some_class instance;
 *
 *  LOG::info("value: {}", log_waive(instance.getter()));
 *
 * Example context:
 *
 *  struct some_class {
 *    //...
 *
 *    std::string getter() { return value_; }
 *
 *  private:
 *    std::string value_;
 *  };
 */
template <typename T> constexpr auto log_waive(T &&what)
{
  // waive the logging overhead check
  return logger::impl::waived_t<T>(std::forward<T>(what));
}

////////////
// either //
////////////

namespace logger::impl {

template <typename WhenTrue, typename WhenFalse> struct either_t {
  constexpr explicit either_t(bool condition, WhenTrue const &when_true, WhenFalse const &when_false)
      : condition_(condition), when_true_(when_true), when_false_(when_false)
  {}

  // make sure this is not used as a non-temporary
  either_t(either_t const &) = delete;
  either_t(either_t &&) = delete;
  either_t &operator=(either_t const &) = delete;
  either_t &operator=(either_t &&) = delete;

  template <typename Out> friend Out &&operator<<(Out &&out, either_t const &what)
  {
    if (what.condition_) {
      out << what.when_true_;
    } else {
      out << what.when_false_;
    }
    return std::forward<Out>(out);
  }

private:
  bool const condition_;
  WhenTrue const &when_true_;
  WhenFalse const &when_false_;
};

} // namespace logger::impl

/**
 * A ternary conditional replacement for logging statements.
 *
 * Motivation:
 * -----------
 *
 * It's not uncommon for logs to have statements like this:
 *
 *  LOG::info("name: {}", condition ? get_name() : "<unknown>")
 *
 * where `get_name` is a run-of-the-mill getter that returns a string reference.
 *
 * This is expected to be a very cheap call, but there's a hidden cost:
 * The ternary operator performs type promotion and comes up with a common type
 * between both `when_true` and `when_false` operands.
 *
 * In this particular example, the common type between `std::string const &` and
 * `char const []` is `std::string const &`. This means that the string literal
 * will, on every log call, be converted to a `std::string` temporary, with dynamic
 * allocation and all, using the cast constructor of `std::string`. Being a temporary,
 * such object will be destroyed and its dynamically allocated memory freed once the
 * expression is fully evaluated.
 *
 * There are not only costs for allocating, deallocating and copying what otherwise
 * should be a compile-time available string, but also (implementation defined)
 * contention on the allocator.
 *
 * With `log_either`, no promotion or allocation is performed and operands are
 * heterogeneously handled by the logger according to their respective types.
 *
 * Example:
 *
 *  // either logs the result of `get_name()` when `condition` is `true`
 *  // or `<unknown>` otherwise
 *  LOG::info("name: {}", log_either(condition, get_name(), "<unknown>"))
 *
 *  // either logs the value pointed to by `ptr` or the string `<null>`
 *  // note that LOG_LAZY prevents the pointer from being dereferenced when it is null
 *  LOG::info("value: {}", log_either(ptr, LOG_LAZY(*ptr), "<null>"));
 */
template <typename WhenTrue, typename WhenFalse> auto log_either(bool condition, WhenTrue &&when_true, WhenFalse &&when_false)
{
  logger::check_logging_overhead<WhenTrue &&, WhenFalse &&>();
  return logger::impl::either_t<WhenTrue, WhenFalse>(
      condition, std::forward<WhenTrue>(when_true), std::forward<WhenFalse>(when_false));
}

////////////////
// surrounded //
////////////////

namespace logger::impl {

template <typename T, char Open, char Close = Open> struct surrounded_t {
  constexpr explicit surrounded_t(T const &what) : what_(what) {}

  // make sure this is not used as a non-temporary
  surrounded_t(surrounded_t const &) = delete;
  surrounded_t(surrounded_t &&) = delete;
  surrounded_t &operator=(surrounded_t const &) = delete;
  surrounded_t &operator=(surrounded_t &&) = delete;

  template <typename Out> friend Out &&operator<<(Out &&out, surrounded_t const &what)
  {
    out << Open << what.what_ << Close;
    return std::forward<Out>(out);
  }

private:
  T const &what_;
};

} // namespace logger::impl

/**
 * A logging helper that surrounds values with an open/close character.
 *
 * Example:
 *
 *  // either logs the string `name: "NAME"` or `name: <unknown>`
 *  LOG::info("name: {}", log_either(condition, log_quoted(get_name()), "<unknown>"));
 *
 *  // logs the string `name: 'NAME'`
 *  LOG::info("name: {}", log_single_quoted(get_name()));
 *
 *  // logs the string `name: *NAME*`
 *  LOG::info("name: {}", log_surround<'*'>(get_name()));
 *
 *  // logs the string `name: <NAME>`
 *  LOG::info("name: {}", log_surround<'<', '>'>(get_name()));
 *
 * Motivation:
 *
 * It helps avoid two problems:
 * - create temporaries when formatting data
 * - (non-goal) cluttering log formats with minor style information
 *
 * The most common scenario where this problem manifests itself is this:
 *
 *  LOG::info("name: {}", condition ? "'" + get_name() + "'" : "<unknown>");
 *
 * where `get_name` is a run-of-the-mill getter that returns a string reference.
 *
 * Two different problems manifest here. One of them is solved by `log_either`.
 * The remaining one is the unnecessary allocation of a temporary string to hold
 * the concatenation resulting from surrounding the name with quotes.
 *
 * `log_surround` solves this problem without any temporaries being allocated by
 * surrounding the value within given `Open` and `Close` characters.
 *
 * There are also convenience alternatives for common formatting:
 * - log_quoted: encloses the value between double quotes;
 * - log_single_quoted: encloses the value between double quotes;
 * - log_parens: encloses the value between parenthesis;
 * - log_backet: encloses the value between square brackets;
 * - log_brace: encloses the value between curly braces.
 */
template <char Open, char Close = Open, typename T> constexpr auto log_surround(T &&what)
{
  logger::check_logging_overhead<T &&>();
  return logger::impl::surrounded_t<T, Open, Close>(std::forward<T>(what));
}

template <typename T> constexpr auto log_quoted(T &&what)
{
  return log_surround<'"'>(std::forward<T>(what));
}

template <typename T> constexpr auto log_single_quoted(T &&what)
{
  return log_surround<'\''>(std::forward<T>(what));
}

template <typename T> constexpr auto log_parens(T &&what)
{
  return log_surround<'(', ')'>(std::forward<T>(what));
}

template <typename T> constexpr auto log_bracket(T &&what)
{
  return log_surround<'[', ']'>(std::forward<T>(what));
}

template <typename T> constexpr auto log_brace(T &&what)
{
  return log_surround<'{', '}'>(std::forward<T>(what));
}

/////////////
// kv_pair //
/////////////

namespace logger::impl {

template <typename Key, typename Value, char Separator, char Open, char Close = Open> struct kv_pair_t {
  static constexpr std::string_view invalid_key_characters = "\"='<>{}[]()?/$#@!";
  using key_t = std::decay_t<Key>;

  constexpr explicit kv_pair_t(Key const &key, Value const &value) : key_(key), value_(value)
  {
    if constexpr (std::is_same_v<key_t, std::string> || std::is_same_v<key_t, std::string_view>) {
      assert(key_.find_first_of(invalid_key_characters) == key_t::npos);
    } else if constexpr (std::is_same_v<key_t, char const *> || std::is_same_v<key_t, char *>) {
      std::string_view const key_view(key_);
      assert(key_view.find_first_of(invalid_key_characters) == std::string_view::npos);
    }
  }

  // make sure this is not used as a non-temporary
  kv_pair_t(kv_pair_t const &) = delete;
  kv_pair_t(kv_pair_t &&) = delete;
  kv_pair_t &operator=(kv_pair_t const &) = delete;
  kv_pair_t &operator=(kv_pair_t &&) = delete;

  template <typename Out> friend Out &&operator<<(Out &&out, kv_pair_t const &what)
  {
    out << what.key_ << Separator << Open << what.value_ << Close;
    return std::forward<Out>(out);
  }

private:
  Key const &key_;
  Value const &value_;
};

} // namespace logger::impl

/**
 * An output stream helper for quoted key/value pairs.
 *
 * Example:
 *
 *  // logs the string `pair: key="value"
 *  LOG::info("pair: {}", log_kv_pair(get_key(), get_value()));
 *
 *  // logs the string `pair: KEY/<VALUE>`
 *  LOG::info("pair: {}", log_kv_pair<'/', '<', '>'>(get_key(), get_value()));
 *
 * Motivation:
 *
 * It helps avoid these problems:
 * - create temporaries when formatting data
 * - leverages compile-time type system checks to find human errors in formatting
 * - (non-goal) cluttering log formats with minor style information
 *
 * The most common scenario where this problem manifests itself is this:
 *
 *  ss << "metric {key=\"" << key << "\"} " << value << ' ' << timestamp << '\n';
 *
 * All the quoting and string formatting becomes pretty cryptic and hard to check
 * with the human eye. Using the helpers there's no formatting necessary.
 */
template <char Separator = '=', char Open = '"', char Close = Open, typename Key, typename Value>
constexpr auto log_kv_pair(Key &&key, Value &&value)
{
  logger::check_logging_overhead<Key &&, Value &&>();
  return logger::impl::kv_pair_t<Key, Value, Separator, Open, Close>(std::forward<Key>(key), std::forward<Value>(value));
}

//////////////
// callable //
//////////////

namespace logger::impl {

template <typename Fn> struct callable_t {
  template <typename UFn> constexpr explicit callable_t(UFn &&fn) : fn_(fn) {}

  // make sure this is not used as a non-temporary
  callable_t(callable_t const &) = delete;
  callable_t(callable_t &&) = delete;
  callable_t &operator=(callable_t const &) = delete;
  callable_t &operator=(callable_t &&) = delete;

  template <typename Out> friend Out &&operator<<(Out &&out, callable_t const &what)
  {
    out << what.fn_();
    return std::forward<Out>(out);
  }

private:
  Fn const &fn_;
};

} // namespace logger::impl

/**
 * A logging helper to allow logging the result of a callable object.
 *
 * The callable is lazily called at the time of logging, and might
 * not be called at all if used in conjunction with conditionals like
 * log_either.
 *
 * Example:
 *
 *  // logs the result of calling the given lambda
 *  LOG::info("value: {}", log_call([] { return std::pow(2, 8); }));
 */
template <typename Fn> constexpr auto log_call(Fn &&callable)
{
  return logger::impl::callable_t<Fn>(callable);
}

/**
 * A logging helper to allow lazy evaluation of an expression.
 *
 * Example:
 *
 *  // either logs the value pointed to by `ptr` or the string `<null>`
 *  LOG::info("value: {}", log_either(ptr, LOG_LAZY(*ptr), "<null>"));
 */
#define LOG_LAZY(...)                                                                                                          \
  ::log_call(                                                                                                                  \
      [&]() -> ::std::conditional_t<                                                                                           \
                ::std::is_reference_v<decltype(__VA_ARGS__)>,                                                                  \
                decltype(__VA_ARGS__) const &,                                                                                 \
                decltype(__VA_ARGS__)> { return __VA_ARGS__; })
