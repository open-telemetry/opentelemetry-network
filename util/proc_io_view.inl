// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <platform/types.h>

// from proc(5) - /proc/[pid]/io
#define PROC_IO_VIEW_IMPL(X)                                                                                                   \
  X(views::NumberView<u64>, rchar, "rchar")                                                                                    \
  X(views::NumberView<u64>, wchar, "wchar")                                                                                    \
  X(views::NumberView<u64>, syscr, "syscr")                                                                                    \
  X(views::NumberView<u64>, syscw, "syscw")                                                                                    \
  X(views::NumberView<u64>, read_bytes, "read_bytes")                                                                          \
  X(views::NumberView<u64>, write_bytes, "write_bytes")                                                                        \
  X(views::NumberView<u64>, cancelled_write_bytes, "cancelled_write_bytes")
