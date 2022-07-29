/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <exception>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <variant>

#include <cassert>
#include <cstdlib>

/**
 * Expected: a wrapper of return values for return-based error handling.
 */

////////////////////
// error policies //
////////////////////

/**
 * An error policy for `Expected` for throwing unexpected errors (e.g.:
 * exceptions).
 */
template <typename Error> class ThrowableExpectedErrorPolicy {
public:
  using error_type = Error;

  /**
   * Throws the given error.
   *
   * This function should not return.
   */
  [[noreturn]] static void raise(error_type const &error) { throw error; }

  /**
   * Tells whether this is a throwing error policy or not.
   */
  static constexpr bool throwing() { return true; }
};

/**
 * An error policy for `Expected` for value-based errors (e.g.: error codes)
 * which doesn't support throwing..
 */
template <typename Error> class ErrorValueExpectedErrorPolicy {
public:
  using error_type = Error;

  /**
   * Tells whether this is a throwing error policy or not.
   */
  static constexpr bool throwing() { return false; }
};

/**
 * Resolves to either `ThrowableExpectedErrorPolicy` if `Error` is a subclass of
 * `std::exception` or `ErrorValueExpectedErrorPolicy` otherwise.
 */
template <typename Error>
using DefaultExpectedErrorPolicy = std::conditional_t<
    std::is_base_of_v<std::exception, Error>,
    ThrowableExpectedErrorPolicy<Error>,
    ErrorValueExpectedErrorPolicy<Error>>;

////////////////////
// check policies //
////////////////////

/**
 * A check policy for `Expected` that asserts pre-conditions.
 */
class AssertExpectedCheckPolicy {
public:
  /**
   * Asserts that `expected` contains a value.
   */
  template <typename Expected> static void check_is_value(Expected const &expected) { assert(expected.has_value()); }

  /**
   * Asserts that `expected` contains an error.
   */
  template <typename Expected> static void check_is_error(Expected const &expected) { assert(!expected.has_value()); }
};

/**
 * A check policy for `expected` that throws on failed pre-conditions.
 */
class ThrowingExpectedCheckPolicy {
public:
  template <typename Expected>
  /**
   * Checks that `expected` contains a value and throws a `std::logic_error` if
   * not.
   */
  static void check_is_value(Expected const &expected)
  {
    if (!expected.has_value()) {
      throw std::logic_error("attempting to retrieve the value out of an unexpected error");
    }
  }

  /**
   * Checks that `expected` contains an error and throws a `std::logic_error` if
   * not.
   */
  template <typename Expected> static void check_is_error(Expected const &expected)
  {
    if (expected.has_value()) {
      throw std::logic_error("attempting to retrieve the error out of an expected value");
    }
  }
};

using DefaultExpectedCheckPolicy = AssertExpectedCheckPolicy;

/**
 * A tag for unexcpected error conditions.
 *
 * Used in conjunction with `Expected`.
 */
class Unexpected {
};

constexpr Unexpected unexpected = {};

template <typename, typename...> struct is_safe_overload : std::true_type {
};

template <typename Class, typename T>
struct is_safe_overload<Class, T>
    : std::integral_constant<
          bool,
          !std::is_base_of<Class, typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value> {
};

/**
 * A wrapper of return values for return-based error handling.
 *
 * Allows the representation of both expected return values and unexpected error
 * conditions.
 */
template <
    typename T,
    typename Error,
    typename CheckPolicy = DefaultExpectedCheckPolicy,
    template <typename> typename ErrorPolicy = DefaultExpectedErrorPolicy>
class Expected {
public:
  using value_type = T;
  using error_type = Error;
  using error_policy = ErrorPolicy<Error>;
  using check_policy = CheckPolicy;

private:
  class ErrorContainer {
  public:
    template <typename... UArgs, typename = std::enable_if_t<is_safe_overload<ErrorContainer, UArgs...>::value>>
    explicit ErrorContainer(UArgs &&... args) : error_(std::forward<UArgs>(args)...)
    {}

    ErrorContainer(ErrorContainer const &) = default;
    ErrorContainer(ErrorContainer &&) = default;

    /**
     * Getter for the wrapped error.
     */
    auto const &get() const { return error_; }

    /**
     * Getter for the wrapped error.
     */
    auto &get() { return error_; }

  private:
    error_type error_;
  };

public:
  /**
   * Constructs an `Expected` with inplace construction of value from the given
   * arguments.
   */
  /* implicit */
  template <
      typename... UArgs,
      typename = std::enable_if_t<is_safe_overload<Expected, UArgs...>::value>,
      typename = std::enable_if_t<sizeof...(UArgs) != 1 || !(std::is_same_v<Unexpected, UArgs> && ... && true)>>
  Expected(UArgs &&... args) : data_(std::in_place_type<value_type>, std::forward<UArgs>(args)...)
  {}

  /**
   * Constructs an `Expected` with an unexpected error.
   */
  template <typename... UArgs>
  Expected(Unexpected, UArgs &&... args) : data_(std::in_place_type<ErrorContainer>, std::forward<UArgs>(args)...)
  {}

  Expected(Expected const &) = default;
  Expected(Expected &&) = default;

  /**
   * Getter for the wrapped value.
   *
   * Calls the check policy to ensure this object contains a value.
   */
  value_type const &value() const
  {
    check_policy::check_is_value(*this);
    return std::get<value_type>(data_);
  }

  /**
   * Getter for the wrapped value.
   *
   * Calls the check policy to ensure this object contains a value.
   */
  value_type &value()
  {
    check_policy::check_is_value(*this);
    return std::get<value_type>(data_);
  }

  /**
   * Conditional getter for the wrapped value.
   *
   * Returns a pointer to the value if present, or `nullptr` otherwise.
   */
  value_type const *try_value() const
  {
    if (!has_value()) {
      return nullptr;
    }
    return std::addressof(value());
  }

  /**
   * Conditional getter for the wrapped value.
   *
   * Returns a pointer to the value if present, or `nullptr` otherwise.
   */
  value_type *try_value()
  {
    if (!has_value()) {
      return nullptr;
    }
    return std::addressof(value());
  }

  /**
   * Getter for the wrapped error.
   *
   * Calls the check policy to ensure this object contains an error.
   */
  error_type const &error() const
  {
    check_policy::check_is_error(*this);
    return std::get<ErrorContainer>(data_).get();
  }

  /**
   * Getter for the wrapped error object.
   *
   * Calls the check policy to ensure this object contains an error.
   */
  error_type &error()
  {
    check_policy::check_is_error(*this);
    return std::get<ErrorContainer>(data_).get();
  }

  /**
   * Conditional getter for the wrapped error.
   *
   * Returns a pointer to the error if present, or `nullptr` otherwise.
   */
  error_type const *try_error() const
  {
    if (has_value()) {
      return nullptr;
    }
    return std::addressof(error());
  }

  /**
   * Conditional getter for the wrapped error.
   *
   * Returns a pointer to the error if present, or `nullptr` otherwise.
   */
  error_type *try_error()
  {
    if (has_value()) {
      return nullptr;
    }
    return std::addressof(error());
  }

  /**
   * Returns `true` if a value is present, otherwise returns `false` as an error
   * is.
   */
  bool has_value() const { return !data_.index(); }

  /**
   * Stores a value in-place constructed from the given arguments.
   */
  template <typename... UArgs> Expected &set_value(UArgs &&... args)
  {
    data_.template emplace<0>(std::forward<UArgs>(args)...);
    return *this;
  }

  /**
   * Stores an error in-place constructed from the given arguments.
   */
  template <typename... UArgs> Expected &set_error(UArgs &&... args)
  {
    data_.template emplace<1>(std::forward<UArgs>(args)...);
    return *this;
  }

  /**
   * Calls the given function `fn` if a value is present, otherwise doesn't do
   * anything.
   *
   * `fn` will be given the value as its sole argument if such overload exists.
   */
  template <typename Fn> Expected const &on_value(Fn &&fn) const
  {
    if (has_value()) {
      std::forward<Fn>(fn)(value());
    }

    return *this;
  }

  /**
   * Calls the given function `fn` if a value is present, otherwise doesn't do
   * anything.
   *
   * `fn` will be given the value as its sole argument if such overload exists.
   */
  template <typename Fn> Expected &on_value(Fn &&fn)
  {
    if (has_value()) {
      std::forward<Fn>(fn)(value());
    }

    return *this;
  }

  /**
   * Calls the given function `fn` if an error is present, otherwise doesn't do
   * anything.
   *
   * `fn` will be given the error as its sole argument if such overload exists.
   */
  template <typename Fn> Expected const &on_error(Fn &&fn) const
  {
    if (!has_value()) {
      std::forward<Fn>(fn)(error());
    }

    return *this;
  }

  /**
   * Calls the given function `fn` if an error is present, otherwise doesn't do
   * anything.
   *
   * `fn` will be given the error as its sole argument if such overload exists.
   */
  template <typename Fn> Expected &on_error(Fn &&fn)
  {
    if (!has_value()) {
      std::forward<Fn>(fn)(error());
    }

    return *this;
  }

  /**
   * Returns the value if present, or the result of calling the given function
   * `fn` if an error is present.
   *
   * `fn` will be given the error as its sole argument if such overload exists.
   */
  template <typename Fn> value_type recover(Fn &&fn) const
  {
    if (has_value()) {
      return value();
    }

    return std::forward<Fn>(fn)(error());
  }

  /**
   * Returns the value if present, or the result of calling the given function
   * `fn` if an error is present.
   *
   * `fn` will be given the error as its sole argument if such overload exists.
   */
  template <typename Fn> value_type recover(Fn &&fn)
  {
    if (has_value()) {
      return value();
    }

    return std::forward<Fn>(fn)(error());
  }

  /**
   * Returns the value if present, or the `with` if an error is present.
   */
  template <typename Value> value_type recover_with(Value &&with) const
  {
    if (has_value()) {
      return value();
    }

    return std::forward<Value>(with);
  }

  /**
   * Returns the value if present, or the `with` if an error is present.
   */
  template <typename Value> value_type recover_with(Value &&with)
  {
    if (has_value()) {
      return value();
    }

    return std::forward<Value>(with);
  }

  /**
   * This method can't be called on non-throwable error policies.
   */
  [[noreturn]] void raise() const
  {
    check_policy::check_is_error(*this);
    if constexpr (throwing()) {
      error_policy::raise(error());
    } else {
      static_assert(throwing(), "can't raise on a non-throwing error policy");
    }

    // ensure we won't return in case of a misbehaving error policy
    std::abort();
  }

  Expected const &try_raise() const
  {
    if (has_value()) {
      return *this;
    }
    raise();
  }

  Expected &try_raise()
  {
    if (has_value()) {
      return *this;
    }
    raise();
  }

  value_type const &operator*() const { return value(); }
  value_type &operator*() { return value(); }

  value_type const *operator->() const { return try_value(); }
  value_type *operator->() { return try_value(); }

  explicit operator bool() const { return has_value(); }
  bool operator!() const { return !has_value(); }

  Expected &operator=(Expected const &rhs)
  {
    if (rhs.has_value()) {
      set_value(rhs.value());
    } else {
      set_error(rhs.error());
    }
    return *this;
  }

  Expected &operator=(Expected &&rhs)
  {
    if (rhs.has_value()) {
      set_value(std::move(rhs.value()));
    } else {
      set_error(std::move(rhs.error()));
    }
    return *this;
  }

  constexpr static bool throwing() { return error_policy::throwing(); }

private:
  std::variant<value_type, ErrorContainer> data_;
};

/**
 * Convenient alias for an `Expected` that uses `ThrowingExpectedCheckPolicy`.
 */
template <typename T, typename Error, template <typename> typename ErrorPolicy = DefaultExpectedErrorPolicy>
using CheckedExpected = Expected<T, Error, ThrowingExpectedCheckPolicy, ErrorPolicy>;

// specializations

template <> class ErrorValueExpectedErrorPolicy<std::error_code> : public ThrowableExpectedErrorPolicy<std::error_code> {
public:
  [[noreturn]] static void raise(error_type const &error) { throw std::system_error(error); }
};

template <> class ErrorValueExpectedErrorPolicy<std::errc> : public ThrowableExpectedErrorPolicy<std::errc> {
public:
  [[noreturn]] static void raise(error_type const &error)
  {
    throw std::system_error{static_cast<std::underlying_type_t<error_type>>(error), std::generic_category()};
  }
};
