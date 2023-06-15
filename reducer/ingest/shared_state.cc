// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "shared_state.h"

#include <generated/ebpf_net/ingest/index.h>

namespace reducer::ingest {

namespace {

struct GlobalState {
  ThreadSafeMap<IPv6Address, IPv6Address> private_to_public_address_map;
};

struct LocalState {
  ::ebpf_net::ingest::Index *index = nullptr;
  NpmConnection *connection = nullptr;
  ::ebpf_net::ingest::auto_handles::logger *logger = nullptr;
  ::ebpf_net::ingest::auto_handles::core_stats *core_stats = nullptr;
  ::ebpf_net::ingest::auto_handles::ingest_core_stats *ingest_core_stats = nullptr;
};

GlobalState *global_state()
{
  static GlobalState state;
  return &state;
}

LocalState *local_state()
{
  thread_local LocalState state;
  return &state;
}

} // namespace

ThreadSafeMap<IPv6Address, IPv6Address> &global_private_to_public_address_map()
{
  return global_state()->private_to_public_address_map;
}

::ebpf_net::ingest::Index *local_index()
{
  assert(local_state()->index != nullptr);
  return local_state()->index;
}

NpmConnection *local_connection()
{
  assert(local_state()->connection != nullptr);
  return local_state()->connection;
}

::ebpf_net::ingest::weak_refs::logger local_logger()
{
  assert(local_state()->logger != nullptr);
  return *(local_state()->logger);
}

::ebpf_net::ingest::weak_refs::core_stats local_core_stats_handle()
{
  assert(local_state()->core_stats != nullptr);
  return *(local_state()->core_stats);
}

::ebpf_net::ingest::weak_refs::ingest_core_stats local_ingest_core_stats_handle()
{
  assert(local_state()->ingest_core_stats != nullptr);
  return *(local_state()->ingest_core_stats);
}

void set_local_index(::ebpf_net::ingest::Index *const index)
{
  local_state()->index = index;
}

void set_local_connection(NpmConnection *const connection)
{
  local_state()->connection = connection;
}

void set_local_logger(::ebpf_net::ingest::auto_handles::logger *logger)
{
  local_state()->logger = logger;
}

void set_local_core_stats_handle(::ebpf_net::ingest::auto_handles::core_stats *core_stats)
{
  local_state()->core_stats = core_stats;
}

void set_local_ingest_core_stats_handle(::ebpf_net::ingest::auto_handles::ingest_core_stats *ingest_core_stats)
{
  local_state()->ingest_core_stats = ingest_core_stats;
}

} // namespace reducer::ingest
