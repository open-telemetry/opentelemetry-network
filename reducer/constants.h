/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/constants.h>
#include <platform/types.h>
#include <util/enum.h>
#include <util/version.h>

#include <absl/time/time.h>

#include <cassert>
#include <cstdint>
#include <string_view>

#define ENUM_NAME NodeResolutionType
#define ENUM_TYPE u8
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(NONE, 0, "")                                                                                                               \
  X(IP, 1, "")                                                                                                                 \
  X(DNS, 2, "")                                                                                                                \
  X(AWS, 3, "")                                                                                                                \
  X(INSTANCE_METADATA, 4, "")                                                                                                  \
  X(PROCESS, 5, "")                                                                                                            \
  X(LOCALHOST, 6, "")                                                                                                          \
  X(K8S_CONTAINER, 7, "")                                                                                                      \
  X(CONTAINER, 8, "")                                                                                                          \
  X(NOMAD, 9, "")
#define ENUM_DEFAULT NONE
#include <util/enum_operators.inl>

constexpr std::size_t K8S_UID_SUFFIX_LENGTH = 64;

enum class FlowSide : bool {
  SIDE_A,
  SIDE_B,
};

enum class UpdateDirection {
  NONE,
  A_TO_B,
  B_TO_A,
};

static constexpr char kUnknown[] = "(unknown)";

static constexpr char kCommKubelet[] = "kubelet";

static constexpr u16 kPortDNS = 53;

static constexpr char kProductIdDimName[] = "sf_product";
static constexpr char kProductIdDimValue[] = "network-explorer";
static constexpr char kServiceName[] = "reducer";

enum class ConnectorDirection : uint8_t {
  UNKNOWN = 0,
  REQUEST = 1,
  RESPONSE = 2,
};

// Environment variable specifying which directory to use for saving data at runtime.
static constexpr auto DATA_DIR_VAR = "EBPF_NET_DATA_DIR";

// Environment variable specifying the path to a GeoIP database file.
static constexpr char GEOIP_PATH_VAR[] = "GEOIP_PATH";

// Time that RPC handlers wait between checks for messages in message queues.
static constexpr auto RPC_HANDLE_TIME = 20ms;

// Maximum number of messages each RPC handlers handles from each queue in each call
static constexpr auto kMaxRpcBatchPerQueue = 10 * 1000;

// helper functions

// Reverse the connector direction.
// @returns: UNKNOWN on UNKNOWN, and flips REQUEST and RESPONSE
inline ConnectorDirection reverse(ConnectorDirection direction);

// unary plus operator (neutral integer operator) to cast the enum class into
// the underlying type (say, for indexing)
inline std::underlying_type_t<FlowSide> operator+(FlowSide side);

// complement operator that retrieves the other side
inline FlowSide operator~(FlowSide side);

inline FlowSide u8_to_side(u8 side);

inline std::string_view to_string(FlowSide side, std::string_view fallback = "<unknown>");

template <typename Out> Out &operator<<(Out &&out, FlowSide side);

namespace versions::client {

// minimum version of a collector for incoming connections to be accepted
constexpr VersionInfo MINIMUM_ACCEPTED_VERSION{0, 9, 0};

} // namespace versions::client

#include <reducer/constants.inl>
