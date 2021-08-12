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

#include <util/system_ops.h>

#include <sys/resource.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

Expected<struct ::sysinfo, std::system_error> system_info()
{
  struct ::sysinfo info;

  if (::sysinfo(&info)) {
    return {unexpected, errno, std::generic_category()};
  }

  return info;
}

Expected<ResourceUsage, std::system_error> get_resource_usage()
{
  struct ::rusage info;
  struct ::rusage children;

  if (::getrusage(RUSAGE_SELF, &info)) {
    return {unexpected, errno, std::generic_category()};
  }

  if (::getrusage(RUSAGE_CHILDREN, &children)) {
    return {unexpected, errno, std::generic_category()};
  }

  return ResourceUsage{
      .user_mode_time = std::chrono::seconds(info.ru_utime.tv_sec + children.ru_utime.tv_sec) +
                        std::chrono::microseconds(info.ru_utime.tv_usec + children.ru_utime.tv_usec),
      .kernel_mode_time = std::chrono::seconds(info.ru_stime.tv_sec + children.ru_stime.tv_sec) +
                          std::chrono::microseconds(info.ru_stime.tv_usec + children.ru_stime.tv_usec),
      .max_resident_set_size = static_cast<std::uint64_t>(info.ru_maxrss + children.ru_maxrss) * 1024,
      .minor_page_faults = static_cast<std::uint32_t>(info.ru_minflt + children.ru_minflt),
      .major_page_faults = static_cast<std::uint32_t>(info.ru_majflt + children.ru_majflt),
      .block_input_count = static_cast<std::uint32_t>(info.ru_inblock + children.ru_inblock),
      .block_output_count = static_cast<std::uint32_t>(info.ru_oublock + children.ru_oublock),
      .voluntary_context_switch_count = static_cast<std::uint32_t>(info.ru_nvcsw + children.ru_nvcsw),
      .involuntary_context_switch_count = static_cast<std::uint32_t>(info.ru_nivcsw + children.ru_nivcsw),
      .cpu_usage_by_process = 0};
}

Expected<struct ::utsname, std::system_error> get_host_info()
{
  struct ::utsname info;

  if (::uname(&info)) {
    return {unexpected, errno, std::generic_category()};
  }

  return info;
}

Expected<std::string, std::error_code> get_host_name(std::size_t max_length)
{
  std::string hostname(max_length, '\0');
  if (auto const size = ::gethostname(hostname.data(), hostname.size())) {
    return {unexpected, std::error_code{errno, std::generic_category()}};
  } else {
    hostname.resize(strnlen(hostname.data(), hostname.size()));
    return std::move(hostname);
  }
}

Expected<std::size_t, std::error_code> memory_page_size()
{
  if (auto const result = sysconf(_SC_PAGESIZE); result != -1) {
    return static_cast<std::size_t>(result);
  }
  return {unexpected, std::error_code{errno, std::generic_category()}};
}
