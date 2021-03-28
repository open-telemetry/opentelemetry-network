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

#define PROCESS_HANDLER_CPU_MEM_IO_FIELDS(Fn) \
  Fn(user_time_ns, u64, data::CounterToRate) \
  Fn(system_time_ns, u64, data::CounterToRate) \
  \
  Fn(thread_count, u32, data::Gauge) \
  \
  Fn(resident_pages_file_mapping, u64, data::Gauge) \
  Fn(resident_pages_anonymous, u64, data::Gauge) \
  Fn(resident_pages_shared_memory, u64, data::Gauge) \
  \
  Fn(minor_page_faults, u64, data::CounterToRate) \
  Fn(major_page_faults, u64, data::CounterToRate) \
  \
  Fn(block_io_delay_ns, u64, data::CounterToRate) \
  Fn(block_io_delay_count, u32, data::CounterToRate) \
  \
  Fn(swap_in_delay_ns, u64, data::CounterToRate) \
  Fn(swap_in_delay_count, u32, data::CounterToRate) \
  \
  Fn(free_pages_delay_ns, u64, data::CounterToRate) \
  Fn(free_pages_delay_count, u32, data::CounterToRate) \
  \
  Fn(thrashing_page_delay_ns, u64, data::CounterToRate) \
  Fn(thrashing_page_delay_count, u32, data::CounterToRate) \
  \
  Fn(read_syscalls, u64, data::CounterToRate) \
  Fn(write_syscalls, u64, data::CounterToRate) \
  \
  Fn(bytes_logically_read, u64, data::CounterToRate) \
  Fn(bytes_logically_written, u64, data::CounterToRate) \
  \
  Fn(bytes_physically_read, u64, data::CounterToRate) \
  Fn(bytes_physically_written, u64, data::CounterToRate) \
  \
  Fn(cancelled_write_bytes, u64, data::CounterToRate) \
  \
  Fn(voluntary_context_switches, u64, data::CounterToRate) \
  Fn(involuntary_context_switches, u64, data::CounterToRate)
