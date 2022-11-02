// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/matching/aws_enrichment_span.h>

#include <reducer/constants.h>

#include <util/log.h>

namespace reducer::matching {

AwsEnrichmentSpan::AwsEnrichmentSpan() {}

AwsEnrichmentSpan::~AwsEnrichmentSpan() {}

void AwsEnrichmentSpan::aws_enrichment(
    ::ebpf_net::matching::weak_refs::aws_enrichment span_ref, u64 timestamp, jsrv_matching__aws_enrichment *msg)
{
  LOG::trace_in(
      std::make_tuple(NodeResolutionType::AWS, ClientType::cloud),
      "matching::AwsEnrichmentSpan::aws_enrichment:"
      " role='{}' az='{}' id='{}'",
      msg->role,
      msg->az,
      msg->id);

  if (!info_) {
    info_.emplace();
  }

  assign_jb(info_->role, msg->role);
  assign_jb(info_->az, msg->az);
  assign_jb(info_->id, msg->id);
}

} // namespace reducer::matching
