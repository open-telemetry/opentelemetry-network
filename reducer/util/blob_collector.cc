// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/util/blob_collector.h>

#include <common/component.h>
#include <util/log.h>
#include <util/log_formatters.h>

namespace {
constexpr auto COOLDOWN = 2s;
} // namespace

void BlobCollector::collect(
    std::underlying_type_t<CollectedBlobType> raw_type, u64 subtype, std::string_view metadata, std::string_view blob)
{
  auto const type = sanitize_enum(static_cast<CollectedBlobType>(raw_type));
  auto const index = enum_index_of(type);

  if (!rate_counters_[index].elapsed(COOLDOWN)) {
    return;
  }

  rate_counters_[index].reset();

  switch (type) {
  default:
    // TODO: this should be saved in appropriate storage, akin to minidumps
    //       for now just log so we can tune the parser
    LOG::warn("collected blob `{}` (subtype={} metadata={}): ```{}```", type, subtype, metadata, blob);
    break;
  }
}
