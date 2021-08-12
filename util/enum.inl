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

#include <utility>

#include <cassert>

template <typename Enum> constexpr typename EnumSet<Enum>::int_type EnumSet<Enum>::mask(Enum value)
{
  return traits::is_valid(value) ? make_bit<int_type>(enum_index_of(value)) : int_type{0};
}

template <typename Enum> constexpr EnumSet<Enum> &EnumSet<Enum>::add(Enum value)
{
  set_ |= mask(value);
  return *this;
}

template <typename Enum> constexpr EnumSet<Enum> &EnumSet<Enum>::add(EnumSet set)
{
  set_ |= set.set_;
  return *this;
}

template <typename Enum> constexpr bool EnumSet<Enum>::contains(Enum value) const
{
  return set_ & mask(value);
}

template <typename Enum> constexpr bool EnumSet<Enum>::contains(EnumSet set) const
{
  return (set_ & set.set_) == set.set_;
}

template <typename Enum> typename EnumSet<Enum>::const_iterator &EnumSet<Enum>::const_iterator::operator++()
{
  set_ = disable_least_significant_bit(set_);
  return *this;
}

template <typename Enum> typename EnumSet<Enum>::const_iterator EnumSet<Enum>::const_iterator::operator++(int)
{
  auto copy = *this;
  ++*this;
  return copy;
}

template <typename Enum> constexpr Enum EnumSet<Enum>::const_iterator::operator*() const
{
  assert(set_);
  return traits::values[least_significant_bit_index(set_)];
}
