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

template <typename Out> Out &&operator<<(Out &&out, K8sMetadata const &what)
{
  out << "\"container_name\":\"" << what.container_name_ << "\",\"pod_name\":\"" << what.pod_name_ << "\",\"pod_ns\":\""
      << what.pod_ns_ << "\",\"pod_uid\":\"" << what.pod_uid_ << "\",\"sandbox_uid\":\"" << what.sandbox_uid_
      << "\",\"ports\":[";
  bool first = true;
  for (auto const &port : what.ports_) {
    if (first) {
      first = false;
    } else {
      out << ',';
    }
    out << '"' << port.second.port << "\":{\"name\":\"" << port.second.name << "\",\"protocol\":\"" << port.second.protocol
        << "\"}";
  }
  out << ']';

  return std::forward<Out>(out);
}
