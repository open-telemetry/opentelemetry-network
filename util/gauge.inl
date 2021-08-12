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

namespace data {

template <typename T> template <typename Out> Out Gauge<T>::average() const
{
  switch (count_) {
  case 0:
    return Out{};
  case 1:
    return static_cast<Out>(sum_);
  default:
    return static_cast<Out>(sum_) / count_;
  }
}

template <typename T> Gauge<T> &Gauge<T>::operator+=(T const &value)
{
  changed_ = value_ != value;
  value_ = value;

  if (empty()) {
    min_ = value;
    max_ = value;
  } else {
    if (value < min_) {
      min_ = value;
    }
    if (value > max_) {
      max_ = value;
    }
  }

  sum_ += value;
  ++count_;

  return *this;
}

template <typename T> Gauge<T> &Gauge<T>::operator+=(Gauge const &value)
{
  changed_ = value_ != value.value_ || min_ != value.min_ || max_ != value.max_ || sum_ != value.sum_ || count_ != value.count_;

  value_ = value.value_;

  if (value.min_ < min_) {
    min_ = value.min_;
  }
  if (value.max_ > max_) {
    max_ = value.max_;
  }
  sum_ += value.sum_;
  count_ += value.count_;

  return *this;
}

template <typename T> void Gauge<T>::reset()
{
  value_ = {};
  min_ = {};
  max_ = {};
  sum_ = {};
  count_ = 0;
}

template <typename Out, typename T> Out &&operator<<(Out &&out, Gauge<T> const &value)
{
  out << "{value=" << value.value() << " min=" << value.min() << " max=" << value.max() << " avg=" << value.average() << '}';

  return std::forward<Out>(out);
}

} // namespace data
