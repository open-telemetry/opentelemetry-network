/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/expected.h>

#include <system_error>
#include <thread>

/**
 * Sets the current thread name to the given string.
 *
 * The thread name has a limitation of 15 characters, so it will be truncated
 * accordingly.
 *
 * Returns true on success or an error code on failure.
 */
Expected<bool, std::errc> set_self_thread_name(std::string_view name);
