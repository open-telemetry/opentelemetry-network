/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

constexpr std::string_view release_mode_string =
#ifdef NDEBUG
    "release"
#else  // NDEBUG
    "debug"
#endif // NDEBUG
    ;

constexpr bool is_release_mode =
#ifdef NDEBUG
    true
#else  // NDEBUG
    false
#endif // NDEBUG
    ;
