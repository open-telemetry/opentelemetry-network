// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/random.h>

template <typename LowerBound, typename UpperBound>
std::common_type_t<LowerBound, UpperBound> compute_jitter(LowerBound jitter_lower_bound, UpperBound jitter_upper_bound)
{
  using finer_grain_duration = std::common_type_t<LowerBound, UpperBound>;
  return finer_grain_duration{RNG<std::make_unsigned_t<typename finer_grain_duration::rep>>::next(
      jitter_lower_bound.count(), jitter_upper_bound.count())};
}

template <typename TimePointRep, typename TimePointPeriod, typename LowerBound, typename UpperBound>
std::common_type_t<std::chrono::duration<TimePointRep, TimePointPeriod>, LowerBound, UpperBound> add_jitter(
    std::chrono::duration<TimePointRep, TimePointPeriod> timepoint,
    LowerBound jitter_lower_bound,
    UpperBound jitter_upper_bound)
{
  return timepoint + compute_jitter(jitter_lower_bound, jitter_upper_bound);
}

template <typename Clock, typename TimePointDuration, typename LowerBound, typename UpperBound>
std::chrono::time_point<Clock, std::common_type_t<TimePointDuration, LowerBound, UpperBound>> add_jitter(
    std::chrono::time_point<Clock, TimePointDuration> timepoint, LowerBound jitter_lower_bound, UpperBound jitter_upper_bound)
{
  auto duration = add_jitter(timepoint.time_since_epoch(), jitter_lower_bound, jitter_upper_bound);
  return std::chrono::time_point<Clock, decltype(duration)>{duration};
}
