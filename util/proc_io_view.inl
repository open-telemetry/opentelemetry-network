//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
