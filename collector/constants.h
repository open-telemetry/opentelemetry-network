/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/constants.h>
#include <platform/types.h>

constexpr auto HEARTBEAT_INTERVAL = 2s;
constexpr auto WRITE_BUFFER_SIZE = 16 * 1024;
