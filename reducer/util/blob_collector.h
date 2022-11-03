/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/collected_blob_type.h>
#include <util/enum.h>
#include <util/stop_watch.h>

class BlobCollector {
public:
  void
  collect(std::underlying_type_t<CollectedBlobType> raw_type, u64 subtype, std::string_view metadata, std::string_view blob);

private:
  enum_traits<CollectedBlobType>::array_map<StopWatch<>> rate_counters_;
};
