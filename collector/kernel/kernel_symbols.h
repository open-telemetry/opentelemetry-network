// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <absl/container/flat_hash_set.h>

#include <istream>
#include <string>
#include <string_view>

using KernelSymbols = absl::flat_hash_set<std::string>;

// Reads and parses kernel symbol names.
// Throws an exception on parsing error.
KernelSymbols read_proc_kallsyms(std::istream &stream);

// Reads kernel symbol names from a special file in the proc filesystem.
// Throws an exception if the file can not be read and its content parsed.
KernelSymbols read_proc_kallsyms(const char *path = "/proc/kallsyms");
