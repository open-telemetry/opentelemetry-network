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
