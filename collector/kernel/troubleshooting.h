/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <collector/kernel/troubleshoot_item.h>
#include <common/host_info.h>
#include <util/logger.h>

/**
 * Prints a troubleshooting message for the given item.
 *
 * In case the item is unrecoverable, calls `exit()` and doesn't return.
 */
void print_troubleshooting_message_and_exit(
    HostInfo const &info,
    TroubleshootItem item,
    std::exception const &e,
    std::optional<std::reference_wrapper<logging::Logger>> log = std::nullopt,
    std::function<void()> flush_and_close = nullptr);
