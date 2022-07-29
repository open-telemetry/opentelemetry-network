// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "util/tdigest.h"

#include <algorithm>
#include <assert.h>
#include <thread>
namespace util {
namespace {

thread_local TDigestMerger default_merger;

double k_to_q(double k, double d)
{
  double k_div_d = k / d;
  if (k_div_d >= 0.5) {
    double base = 1 - k_div_d;
    return 1 - 2 * base * base;
  } else {
    return 2 * k_div_d * k_div_d;
  }
}

double clamp(double v, double lo, double hi)
{
  if (v > hi) {
    return hi;
  } else if (v < lo) {
    return lo;
  }
  return v;
}
} // namespace

void TDigest::merge_from_values(double *values, u32 value_count)
{
  default_merger.merge_from_values(*this, values, value_count);
}

void TDigest::merge(const TDigest &other)
{
  default_merger.merge(*this, other);
}

double TDigest::estimate_value_at_quantile(double q) const
{
  if (centroid_count_ == 0) {
    return 0.0;
  }

  if (q >= 1.0) {
    return max_;
  }
  if (q <= 0.0) {
    return min_;
  }

  const double rank = q * value_count_;
  double t = 0.0;
  u32 pos = 0;

  t = value_count_ * 1.0;
  pos = centroid_count_;
  for (u32 i = 0; i < centroid_count_; i++) {
    pos--;
    t -= centroids_[pos].weight;
    if (rank >= t) {
      break;
    }
  }
  double delta = 0;
  double min = min_;
  double max = max_;

  if (centroid_count_ > 1) {
    if (pos == 0) {
      delta = centroids_[pos + 1].mean - centroids_[pos].mean;
      max = centroids_[pos + 1].mean;
    } else if (pos == centroid_count_ - 1) {
      delta = centroids_[pos].mean - centroids_[pos - 1].mean;
      min = centroids_[pos - 1].mean;
    } else {
      delta = (centroids_[pos + 1].mean - centroids_[pos - 1].mean) / 2.0;
      min = centroids_[pos - 1].mean;
      max = centroids_[pos + 1].mean;
    }
  }
  auto value = centroids_[pos].mean + ((rank - t) / centroids_[pos].weight - 0.5) * delta;
  return clamp(value, min, max);
}

void TDigestAccumulator::add(double value)
{
  assert(value_count_ < TDIGEST_VALUE_BUFFER_SIZE);
  value_buffer_[value_count_] = value;
  value_count_++;
  if (value_count_ == TDIGEST_VALUE_BUFFER_SIZE) {
    flush();
  }
}

void TDigestAccumulator::flush()
{
  if (value_count_ == 0) {
    return;
  }
  tdigest_.merge_from_values(value_buffer_, value_count_);
  value_count_ = 0;
}

void TDigestMerger::merge_from_values(TDigest &tdigest, double *values, u32 value_count)
{
  if (value_count == 0) {
    return;
  }
  assert(value_count <= TDIGEST_VALUE_BUFFER_SIZE);

  // TODO: use double_radix_sort.
  std::sort(values, (values + value_count));

  tdigest.min_ = (tdigest.value_count_ == 0) ? *values : (std::min(tdigest.min_, *values));
  tdigest.max_ =
      (tdigest.value_count_ == 0) ? *(values + value_count - 1) : (std::max(tdigest.max_, *(values + value_count - 1)));

  tdigest.value_count_ += value_count;
  tdigest.sum_ = 0; // recompute as we go.

  double k_limit = 1;
  double q_limit_times_count = k_to_q(k_limit++, TDIGEST_CENTROID_COUNT_MAX) * tdigest.value_count_;

  Centroid *curr = &merging_buffer_[0];
  Centroid *next = curr + 1;

  const Centroid *sp = tdigest.centroids_;
  const Centroid *sp_end = sp + tdigest.centroid_count_;

  const double *vp = values;
  const double *vp_end = vp + value_count;

  // put the centroid with minimal mean into curr, either from sp or values
  if (sp != sp_end && sp->mean < *vp) {
    *curr = *sp;
    sp++;
  } else {
    curr->mean = *vp;
    curr->weight = 1.0;
    vp++;
  }

  // weights_so_far: total weight up to and including currently considered
  // centroid
  double weights_so_far = curr->weight;

  // quantity to be merged into curr
  double sums_to_merge = 0;
  double weights_to_merge = 0.0;

  while (sp != sp_end || vp != vp_end) {
    // get the next smallest mean into next
    if (sp != sp_end && (vp == vp_end || sp->mean < *vp)) {
      *next = *sp;
      sp++;
    } else {
      next->mean = *vp;
      next->weight = 1.0;
      vp++;
    }

    double next_sum = next->mean * next->weight;
    weights_so_far += next->weight;

    if (weights_so_far <= q_limit_times_count) {
      // will merge this centroid into curr
      sums_to_merge += next_sum;
      weights_to_merge += next->weight;
    } else {
      // close the centroid
      tdigest.sum_ += curr->add(sums_to_merge, weights_to_merge);
      sums_to_merge = 0;
      weights_to_merge = 0;
      curr++;
      next++;
      q_limit_times_count = k_to_q(k_limit++, TDIGEST_CENTROID_COUNT_MAX) * tdigest.value_count_;
    }
  }

  tdigest.sum_ += curr->add(sums_to_merge, weights_to_merge);

  // Deal with floating point precision
  // Switch to insert sort
  std::sort(merging_buffer_, next);

  curr = merging_buffer_;
  Centroid *target = tdigest.centroids_;
  std::copy(merging_buffer_, next, target);

  tdigest.centroid_count_ = next - merging_buffer_;
}

void TDigestMerger::merge(TDigest &left, const TDigest &right)
{
  if (right.value_count_ == 0) {
    return;
  }

  if (left.value_count_ == 0) {
    left = right;
    return;
  }

  left.min_ = std::min(left.min_, right.min_);
  left.max_ = std::max(left.max_, right.max_);

  left.value_count_ += right.value_count_;
  left.sum_ = 0; // recompute as we go.

  double k_limit = 1;
  double q_limit_times_count = k_to_q(k_limit++, TDIGEST_CENTROID_COUNT_MAX) * left.value_count_;

  Centroid *curr = merging_buffer_;
  Centroid *next = curr + 1;

  const Centroid *lp = left.centroids_;
  const Centroid *lp_end = lp + left.centroid_count_;

  const Centroid *rp = right.centroids_;
  const Centroid *rp_end = rp + right.centroid_count_;

  if (lp->mean < rp->mean) {
    *curr = *lp;
    lp++;
  } else {
    *curr = *rp;
    rp++;
  }

  double weights_so_far = curr->weight;
  double sums_to_merge = 0;
  double weights_to_merge = 0.0;

  while (lp != lp_end || rp != rp_end) {
    if (lp != lp_end && (rp == rp_end || lp->mean < rp->mean)) {
      *next = *lp;
      lp++;
    } else {
      *next = *rp;
      rp++;
    }

    double next_sum = next->mean * next->weight;
    weights_so_far += next->weight;

    if (weights_so_far <= q_limit_times_count) {
      sums_to_merge += next_sum;
      weights_to_merge += next->weight;
    } else {
      left.sum_ += curr->add(sums_to_merge, weights_to_merge);
      sums_to_merge = 0;
      weights_to_merge = 0;
      curr++;
      next++;
      q_limit_times_count = k_to_q(k_limit++, TDIGEST_CENTROID_COUNT_MAX) * left.value_count_;
    }
  }

  left.sum_ += curr->add(sums_to_merge, weights_to_merge);

  // Deal with floating point precision
  // Switch to insert_sort
  std::sort(merging_buffer_, next);

  curr = merging_buffer_;
  Centroid *target = left.centroids_;
  std::copy(merging_buffer_, next, target);

  left.centroid_count_ = next - merging_buffer_;
}
} // namespace util
