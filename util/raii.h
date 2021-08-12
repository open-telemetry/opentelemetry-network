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

#include <memory>

/**
 * A `std::unique_ptr` compatible deleter that's statically bound to a free
 * function deleter and is an empty instance.
 */
template <typename T, typename DeleterResult, DeleterResult (*Deleter)(T *)> struct free_function_deleter {
  void operator()(T *p) { Deleter(p); }
};

/**
 * A unique pointer for POD (plain-old-data) that requires deletion using a free function call.
 */
template <typename T, typename DeleterResult, DeleterResult (*Deleter)(T *)>
using pod_unique_ptr = std::unique_ptr<T, free_function_deleter<T, DeleterResult, Deleter>>;
