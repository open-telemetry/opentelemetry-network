/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>

#define AGG_CORE_TRUNCATION_FIELDS(FN)                                                                                         \
  FN(az, 0)                                                                                                                    \
  FN(container, 1)                                                                                                             \
  FN(env, 2)                                                                                                                   \
  FN(id, 3)                                                                                                                    \
  FN(ip, 4)                                                                                                                    \
  FN(ns, 5)                                                                                                                    \
  FN(pod_name, 6)                                                                                                              \
  FN(process, 7)                                                                                                               \
  FN(role, 8)                                                                                                                  \
  FN(version, 9)                                                                                                               \
  FN(role_uid, 10)

namespace reducer::aggregation {

class StatCounters {
public:
  // how many field truncation counters are there
  static constexpr std::size_t field_truncation_count = 0
#define COUNT_FIELD_TRUNCATION_COUNTERS(Field, Index) +1
      AGG_CORE_TRUNCATION_FIELDS(COUNT_FIELD_TRUNCATION_COUNTERS)
#undef COUNT_FIELD_TRUNCATION_COUNTERS
      ;

  // calls the callback with signature (index is i-th counter)
  // `(std::string_view field_name, std::size_t count, std::size_t index)`
  // for each supported field truncation counter
  template <typename Fn>
  void foreach_field_truncation(Fn &&fn) const {
#define CALL_TRUNCATION_FIELD_CALLBACK(Field, Index) fn(#Field, trunc_##Field, Index);
      AGG_CORE_TRUNCATION_FIELDS(CALL_TRUNCATION_FIELD_CALLBACK)
#undef CALL_TRUNCATION_FIELD_CALLBACK
  }

#define DECLARE_TRUNCATION_COUNTER(Field, Index) std::size_t trunc_##Field = 0;
  AGG_CORE_TRUNCATION_FIELDS(DECLARE_TRUNCATION_COUNTER)
#undef DECLARE_TRUNCATION_COUNTER
};

} // namespace reducer::aggregation
