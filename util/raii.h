/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
