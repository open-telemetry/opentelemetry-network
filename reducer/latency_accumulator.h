/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/tdigest.h>

#include <absl/container/flat_hash_map.h>

#include <queue>
#include <string>

namespace reducer {

template <typename Key> class LatencyAccumulator {
public:
  struct PLatencies {
    Key key;
    double p90, p95, p99;
  };

  LatencyAccumulator() : queue_max_size_(30), delta_ns_(10'000'000'000) {}

  const std::vector<PLatencies> &get_p_latencies() const { return latencies_; }

  const absl::flat_hash_map<Key, double> &get_max_latencies() const { return max_latencies_; }

  void add(u64 t, Key key, double latency);

private:
  struct QueueElem {
    u64 t = 0;
    absl::flat_hash_map<Key, util::TDigest> digests;
    absl::flat_hash_map<Key, double> max_latencies;
  };

  void compute_latencies();
  void rotate_window(u64 t);

  const size_t queue_max_size_;
  const u64 delta_ns_;
  std::deque<QueueElem> queue_;
  std::vector<PLatencies> latencies_;
  absl::flat_hash_map<Key, double> max_latencies_;
};

} // namespace reducer

#include "latency_accumulator.inl"
