/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/matching/span_base.h>

#include <reducer/matching/aws_enrichment_info.h>

#include <optional>

namespace reducer::matching {

struct AwsEnrichmentSpan : ebpf_net::matching::AwsEnrichmentSpanBase {
  AwsEnrichmentSpan();
  ~AwsEnrichmentSpan();

  void
  aws_enrichment(::ebpf_net::matching::weak_refs::aws_enrichment span_ref, u64 timestamp, jsrv_matching__aws_enrichment *msg);

  AwsEnrichmentInfo const *info() const { return info_ ? &info_.value() : nullptr; }

private:
  std::optional<AwsEnrichmentInfo> info_;
};

} // namespace reducer::matching
