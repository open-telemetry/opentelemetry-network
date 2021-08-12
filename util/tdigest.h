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
#include "platform/types.h"
#include <stdlib.h>

namespace util {

#define TDIGEST_CENTROID_COUNT_MAX 100
#define TDIGEST_VALUE_BUFFER_SIZE 500

class TDigestAccumulator;
class TDigestMerger;

// TDigests is a biased quantile estimator designed to estimate the values of
// the quantiles of streaming data.
//
// In current implementation, TDigest object can only be modified by the
// associating TDigestAccumulator and TDigestMerger.
class TDigest {
public:
  TDigest() {}
  ~TDigest() {}

  inline double mean() const { return (value_count_ != 0) ? (sum_ / value_count_) : 0.0; }

  inline size_t value_count() const { return value_count_; }
  inline double max() const { return max_; }
  inline double min() const { return min_; }
  inline double sum() const { return sum_; }

  // Estimates the value at given quantile |q|
  double estimate_value_at_quantile(double q) const;

  // Merges |values|.
  // No-op if |value_count| == 0
  void merge_from_values(double *values, u32 value_count);

  // Merges |other| in place.
  // No-op if |other| does not contain any data.
  void merge(const TDigest &other);

private:
  friend class TDigestAccumulator;
  friend class TDigestMerger;

  // Total number of values that this tdigest object has consumed so far.
  size_t value_count_ = 0;

  // Sum, min & max of all values.
  double sum_ = 0.0;
  double min_ = 0.0;
  double max_ = 0.0;

  struct Centroid {
    Centroid() = default;
    ~Centroid() = default;
    Centroid(const Centroid &) = default;
    Centroid(Centroid &&) = default;
    Centroid &operator=(const Centroid &) = default;
    Centroid &operator=(Centroid &&) = default;

    inline bool operator<(const Centroid &other) const { return mean < other.mean; }

    // Adds Centroid{|new_sum|, |new_weight|}, returns resulting sum.
    inline double add(double new_sum, double new_weight)
    {
      new_sum += (mean * weight);
      weight += new_weight;
      mean = new_sum / weight;
      return new_sum;
    }

    double mean = 0.0;
    double weight = 1.0;
  };

  u32 centroid_count_ = 0;
  Centroid centroids_[TDIGEST_CENTROID_COUNT_MAX];
};

// TDigestAccumulator buffers incoming stream data points, and flushes them
// to the associated TDigest object when needed.
//
// It's expected that TDigest object and TDigestAccumulator object are
// 1:1 mapped.
class TDigestAccumulator {
public:
  TDigestAccumulator(TDigest &tdigest) : tdigest_(tdigest) {}
  ~TDigestAccumulator() {}

  // Adds an incoming data point to the internal buffer, flushes them
  // to the associated TDigest object if needed.
  void add(double value);

  // Flushes data points in the internal buffer to the associated TDigest
  // object.
  void flush();

  TDigest &tdigest() { return tdigest_; }

private:
  TDigest &tdigest_;

  u32 value_count_ = 0;
  double value_buffer_[TDIGEST_VALUE_BUFFER_SIZE];
};

// TDigestMerger merges a TDigest object and list of value points, or
// two TDigest objects.
//
// It's expected a TDigestMerger object is shared by multiple
// TDigest/TDigestAcumulator objects. It's caller's responsibilty to ensure
// thread safty.
class TDigestMerger {
public:
  TDigestMerger() {}
  ~TDigestMerger() {}

  // Merges |values| into |tdigest| object.
  // No-op if |value_count| == 0
  void merge_from_values(TDigest &tdigest, double *values, u32 value_count);

  // Merges |target| and |other|, and stores result in |target|.
  // No-op if |other| does not contain any data.
  void merge(TDigest &target, const TDigest &other);

private:
  using Centroid = TDigest::Centroid;

  // Note additional 1 Centroid space for scratching during merging.
  Centroid merging_buffer_[TDIGEST_CENTROID_COUNT_MAX + 1];
};

} // namespace util
