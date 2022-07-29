/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

// This is a helper type for type-matching visitors for std::variants.
// See https://en.cppreference.com/w/cpp/utility/variant/visit

template <class... Ts> struct overloaded_visitor : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded_visitor(Ts...) -> overloaded_visitor<Ts...>;
