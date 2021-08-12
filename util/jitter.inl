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
