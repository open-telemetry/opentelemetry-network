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

template <typename Out> Out &&operator<<(Out &&out, DockerHostConfigMetadata const &what)
{
  out << "\"cpu_shares\":" << what.cpu_shares_ << ",\"cpu_period\":" << what.cpu_period_ << ",\"cpu_quota\":" << what.cpu_quota_
      << ",\"memory_swappiness\":" << static_cast<std::uint16_t>(what.memory_swappiness_)
      << ",\"memory_limit\":" << what.memory_limit_ << ",\"memory_soft_limit\":" << what.memory_soft_limit_
      << ",\"total_memory_limit\":" << what.total_memory_limit_;

  return std::forward<Out>(out);
}
