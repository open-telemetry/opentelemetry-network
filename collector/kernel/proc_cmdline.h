/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>
#include <util/expected.h>

#include <string>
#include <system_error>

// Reads the process command-line from /proc/PID/cmdline.
Expected<std::string, std::error_code> read_proc_cmdline(u32 pid);

// Reads the process command-line from /proc/PID/cmdline.
// On ENOENT (no such file or directory) error, returns an empty string
// instead of an error code.
Expected<std::string, std::error_code> try_read_proc_cmdline(u32 pid);
