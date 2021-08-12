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

/**
 * A class that represents an authz token fetched from the `authz` backend service.
 */

#include <util/expected.h>

#include <chrono>
#include <string>

class AuthzToken {
public:
  using clock = std::chrono::system_clock;
  using duration = clock::time_point::duration;

  AuthzToken(std::string payload, duration issued_at, duration expiration, std::string intake);

  std::string const &payload() const { return payload_; }

  std::string const &intake() const { return intake_; }

  template <typename Duration = duration> Duration issued_at() const
  {
    return std::chrono::duration_cast<Duration>(issued_at_);
  }

  template <typename Duration = duration> Duration expiration() const
  {
    return std::chrono::duration_cast<Duration>(expiration_);
  }

  template <typename Duration = duration, typename R, typename P>
  constexpr Duration time_left(std::chrono::duration<R, P> at) const
  {
    return std::chrono::duration_cast<Duration>(expiration_) - std::chrono::duration_cast<Duration>(at);
  }

  template <typename Duration = duration, typename C, typename D>
  constexpr Duration time_left(std::chrono::time_point<C, D> at) const
  {
    return time_left<Duration>(at.time_since_epoch());
  }

  template <typename R, typename P> constexpr bool has_expired(std::chrono::duration<R, P> at) const
  {
    return time_left(at).count() <= 0;
  }

  template <typename C, typename D> constexpr bool has_expired(std::chrono::time_point<C, D> at) const
  {
    return time_left(at).count() <= 0;
  }

  static Expected<AuthzToken, std::runtime_error> decode_json(std::string_view json);

private:
  std::string payload_;
  duration issued_at_;
  duration expiration_;
  std::string intake_;
};
