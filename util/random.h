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

#pragma once

#include <random>
#include <type_traits>

template <typename T> class RNG {
public:
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);

  using engine_type = std::conditional_t<(sizeof(T) <= sizeof(std::mt19937::result_type)), std::mt19937, std::mt19937_64>;

  using result_type = typename engine_type::result_type;

  static_assert(sizeof(T) <= sizeof(result_type));

  /**
   * Generates a random integer of type `Value` in the range [`lower_bound`, `upper_bound`].
   */
  template <typename Value = result_type> static Value next(Value lower_bound, Value upper_bound);

  /**
   * Generates a random integer of type `Value` in the range [0, `upper_bound`].
   */
  template <typename Value = result_type> static Value next(Value upper_bound);

  /**
   * Generates a random integer of type `Value`.
   */
  template <typename Value = result_type> static Value next();

  static thread_local engine_type engine;
};

#include <util/random.inl>

using rng_32 = RNG<std::uint_fast32_t>;
using rng_64 = RNG<std::uint_fast64_t>;
