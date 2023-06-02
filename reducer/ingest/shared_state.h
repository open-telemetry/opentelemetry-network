/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/ingest/npm_connection.h>
#include <reducer/thread_safe_map.h>

#include <generated/ebpf_net/ingest/index.h>
#include <generated/ebpf_net/logging/writer.h>

#include <util/ip_address.h>

// This library encapuslates a number of shared, mutable global values that are
// accessed from many classes.
// This library is used as a workaround for the inability to effectively pass
// these values through render-generated code. Please, please do not add any
// new values to this class if possible, since doing so makes things like
// testing and behavior isolation much more difficult.

namespace reducer::ingest {

ThreadSafeMap<IPv6Address, IPv6Address> &global_private_to_public_address_map();

// The Index and NpmConnection objects associated with the current thread.
// The returned values are guaranteed to not be null, and will assert-fail if
// they are.
::ebpf_net::ingest::Index *local_index();
NpmConnection *local_connection();
::ebpf_net::ingest::weak_refs::logger local_logger();
::ebpf_net::ingest::weak_refs::core_stats local_core_stats_handle();
::ebpf_net::ingest::weak_refs::ingest_core_stats local_ingest_core_stats_handle();

// Setters for the above values.
void set_local_index(::ebpf_net::ingest::Index *index);
void set_local_connection(NpmConnection *connection);
void set_local_logger(::ebpf_net::ingest::auto_handles::logger *logger);
void set_local_core_stats_handle(::ebpf_net::ingest::auto_handles::core_stats *core_stats);
void set_local_ingest_core_stats_handle(::ebpf_net::ingest::auto_handles::ingest_core_stats *ingest_core_stats);

} // namespace reducer::ingest
