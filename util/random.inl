// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <limits>

#include <cassert>

template <typename T>
thread_local typename RNG<T>::engine_type RNG<T>::engine{[] {
  std::random_device seeder;
  return seeder();
}()};

template <typename T> template <typename Value> Value RNG<T>::next(Value lower_bound, Value upper_bound)
{
  assert(lower_bound <= upper_bound);
  return std::uniform_int_distribution<Value>{lower_bound, upper_bound}(engine);
}

template <typename T> template <typename Value> Value RNG<T>::next(Value upper_bound)
{
  return next<Value>(0, upper_bound);
}

template <typename T> template <typename Value> Value RNG<T>::next()
{
  return next<Value>(std::numeric_limits<Value>::min(), std::numeric_limits<Value>::max());
}
