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
