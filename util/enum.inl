// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
