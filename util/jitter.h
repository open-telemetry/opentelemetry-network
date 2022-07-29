/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <type_traits>

/**
 * Computes a random jitter amount in the range [`jitter_lower_bound`, `jitter_upper_bound`].
 *
 * `{Lower|Upper}Bound` must be an instantiation of class template `std::chrono::duration`.
 *
 * This is a lossless function so the return type has the finer grain between `LowerBound`
 * and `UpperBound`.
 */
template <typename LowerBound, typename UpperBound>
std::common_type_t<LowerBound, UpperBound> compute_jitter(LowerBound jitter_lower_bound, UpperBound jitter_upper_bound);

/**
 * Adjusts the given time point with a random jitter amount in the range
 * [`jitter_lower_bound`, `jitter_upper_bound`].
 *
 * `{Lower|Upper}Bound` must be an instantiation of class template `std::chrono::duration`.
 *
 * `timepoint` must be either of type `std::chrono::duration` or `std::chrono::timepoint`.
 *
 * This is a lossless function so the return type has the finer grain between
 * `timepoint`, `LowerBound` * and `UpperBound`.
 *
 * Example:
 *
 *  timer.defer(add_jitter(timeout, -5s, 5s), [] {
 *    // task code goes here
 *  });
 */
template <typename TimePointRep, typename TimePointPeriod, typename LowerBound, typename UpperBound>
std::common_type_t<std::chrono::duration<TimePointRep, TimePointPeriod>, LowerBound, UpperBound> add_jitter(
    std::chrono::duration<TimePointRep, TimePointPeriod> timepoint,
    LowerBound jitter_lower_bound,
    UpperBound jitter_upper_bound);

template <typename Clock, typename TimePointDuration, typename LowerBound, typename UpperBound>
std::chrono::time_point<Clock, std::common_type_t<TimePointDuration, LowerBound, UpperBound>> add_jitter(
    std::chrono::time_point<Clock, TimePointDuration> timepoint, LowerBound jitter_lower_bound, UpperBound jitter_upper_bound);

#include <util/jitter.inl>
