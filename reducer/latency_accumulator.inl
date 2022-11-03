// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "latency_accumulator.h"

namespace reducer {

template <typename Key> void LatencyAccumulator<Key>::compute_latencies()
{
  latencies_.clear();
  max_latencies_.clear();

  absl::flat_hash_map<Key, util::TDigest> digests;

  for (const auto &t : queue_) {
    for (const auto &[k, v] : t.digests) {
      digests[k].merge(v);
    }
    for (const auto &[k, v] : t.max_latencies) {
      if (max_latencies_[k] < v)
        max_latencies_[k] = v;
    }
  }

  for (const auto &[k, v] : digests) {
    latencies_.push_back(
        {k, v.estimate_value_at_quantile(0.90), v.estimate_value_at_quantile(0.95), v.estimate_value_at_quantile(0.99)});
  }
}

template <typename Key> void LatencyAccumulator<Key>::rotate_window(u64 t)
{
  if (queue_.empty()) {
    queue_.push_back({});
    queue_.back().t = t;
    return;
  }
  auto delta = t - queue_.back().t;
  if (delta > delta_ns_) {
    queue_.push_back({});
    queue_.back().t = t;
    if (queue_.size() > queue_max_size_) {
      queue_.pop_front();
    }
    compute_latencies();
  }
}

template <typename Key> void LatencyAccumulator<Key>::add(u64 t, Key key, double latency)
{
  rotate_window(t);

  auto &digests = queue_.back().digests;

  util::TDigestAccumulator acc(digests[key]);
  acc.add(latency); // millisec
  acc.flush();

  auto &max_latencies = queue_.back().max_latencies;

  if (max_latencies[key] < latency) {
    max_latencies[key] = latency;
  }
}

} // namespace reducer
