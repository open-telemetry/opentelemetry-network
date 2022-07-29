/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <collector/kernel/entrypoint_error.h>
#include <collector/kernel/troubleshoot_item.h>
#include <common/host_info.h>
#include <util/logger.h>

#include <string_view>

constexpr std::string_view CONTACT_INFO_MESSAGE = R"SUPPORT(
To reach Flowmill customer support, please choose one of the contact methods below:

- the shared Slack channel that has been setup between your company and `flowmill.slack.com`;

- email `support@flowmill.com`, prefixing the subject with "Kernel Collector Issue: ".

If possible, please include these logs in the message, along with a description of the problem
you're encountering so we can better support you.
)SUPPORT";

/**
 * Prints a troubleshooting message for the error contained in the given
 * entrypoint info.
 *
 * In case the error is unrecoverable, calls `exit()` and doesn't return.
 */
void print_troubleshooting_message_and_exit(
    HostInfo const &info, EntrypointError error, logging::Logger &log, std::function<void()> flush_and_close);

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
