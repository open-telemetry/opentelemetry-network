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

// from proc(5) - /proc/[pid]/stat
#define PROC_STAT_VIEW_IMPL(X)                                                                                                 \
  X(views::NumberView<int>, pid)                                                                                               \
  X(std::string_view, comm)                                                                                                    \
  X(ProcessState, state)                                                                                                       \
  X(views::NumberView<int>, ppid)                                                                                              \
  X(views::NumberView<int>, pgrp)                                                                                              \
  X(views::NumberView<int>, session)                                                                                           \
  X(views::NumberView<int>, tty_nr)                                                                                            \
  X(views::NumberView<int>, tpgid)                                                                                             \
  X(views::NumberView<unsigned>, flags)                                                                                        \
  X(views::NumberView<unsigned long>, minflt)                                                                                  \
  X(views::NumberView<unsigned long>, cminflt)                                                                                 \
  X(views::NumberView<unsigned long>, majflt)                                                                                  \
  X(views::NumberView<unsigned long>, cmajflt)                                                                                 \
  X(views::NumberView<unsigned long>, utime)                                                                                   \
  X(views::NumberView<unsigned long>, stime)                                                                                   \
  X(views::NumberView<long>, cutime)                                                                                           \
  X(views::NumberView<long>, cstime)                                                                                           \
  X(views::NumberView<long>, priority)                                                                                         \
  X(views::NumberView<long>, nice)                                                                                             \
  X(views::NumberView<long>, num_threads)                                                                                      \
  X(views::NumberView<long>, itrealvalue)                                                                                      \
  X(views::NumberView<unsigned long long>, starttime)                                                                          \
  X(views::NumberView<unsigned long>, vsize)                                                                                   \
  X(views::NumberView<long>, rss)                                                                                              \
  X(views::NumberView<unsigned long>, rsslim)                                                                                  \
  X(views::NumberView<unsigned long>, startcode)                                                                               \
  X(views::NumberView<unsigned long>, endcode)                                                                                 \
  X(views::NumberView<unsigned long>, startstack)                                                                              \
  X(views::NumberView<unsigned long>, kstkesp)                                                                                 \
  X(views::NumberView<unsigned long>, kstkeip)                                                                                 \
  X(views::NumberView<unsigned long>, signal)                                                                                  \
  X(views::NumberView<unsigned long>, blocked)                                                                                 \
  X(views::NumberView<unsigned long>, sigignore)                                                                               \
  X(views::NumberView<unsigned long>, sigcatch)                                                                                \
  X(views::NumberView<unsigned long>, wchan)                                                                                   \
  X(views::NumberView<unsigned long>, nswap)                                                                                   \
  X(views::NumberView<unsigned long>, cnswap)                                                                                  \
  X(views::NumberView<int>, exit_signal)                                                                                       \
  X(views::NumberView<int>, processor)                                                                                         \
  X(views::NumberView<unsigned>, rt_priority)                                                                                  \
  X(views::NumberView<unsigned>, policy)                                                                                       \
  X(views::NumberView<unsigned long long>, delayacct_blkio_ticks)                                                              \
  X(views::NumberView<unsigned long>, guest_time)                                                                              \
  X(views::NumberView<long>, cguest_time)                                                                                      \
  X(views::NumberView<unsigned long>, start_data)                                                                              \
  X(views::NumberView<unsigned long>, end_data)                                                                                \
  X(views::NumberView<unsigned long>, start_brk)                                                                               \
  X(views::NumberView<unsigned long>, arg_start)                                                                               \
  X(views::NumberView<unsigned long>, arg_end)                                                                                 \
  X(views::NumberView<unsigned long>, env_start)                                                                               \
  X(views::NumberView<unsigned long>, env_end)                                                                                 \
  X(views::NumberView<int>, exit_code)
