// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <array>

namespace data {

template <typename T> constexpr T CounterToRate<T>::commit_rate(bool empty_if_unitary)
{
  T choice[2] = {value_ - prev_, T{}};
  prev_ = value_;
  return choice[empty_if_unitary & (count_ < 2)];
}

template <typename T> CounterToRate<T> &CounterToRate<T>::reset()
{
  value_ = {};
  prev_ = {};
  count_ = 0;
  return *this;
}

template <typename T> template <typename U> CounterToRate<T> &CounterToRate<T>::operator+=(U &&value)
{
  value_ = std::forward<U>(value);
  ++count_;
  return *this;
}

template <typename Out, typename U> Out &&operator<<(Out &&out, CounterToRate<U> const &what)
{
  out << "{rate=" << what.peek_rate() << " value=" << what.value() << " prev=" << what.prev() << " count=" << what.count()
      << '}';
  return std::forward<Out>(out);
}

} // namespace data
