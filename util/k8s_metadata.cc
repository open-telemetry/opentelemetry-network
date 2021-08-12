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

#include <util/k8s_metadata.h>

#include <util/json.h>

namespace {
static std::string const CONTAINER_PORT_NAME = "name";
static std::string const CONTAINER_PORT_NUMBER = "containerPort";
static std::string const CONTAINER_PORT_PROTOCOL = "protocol";
} // namespace

K8sMetadata::K8sMetadata(nlohmann::json const &labels)
    : container_name_(get_string_view(labels, CONTAINER_NAME)),
      pod_name_(get_string_view(labels, POD_NAME)),
      pod_ns_(get_string_view(labels, POD_NS)),
      pod_uid_(get_string_view(labels, POD_UID)),
      sandbox_uid_(get_string_view(labels, SANDBOX_ID))
{
  if (auto const ports_string = try_get_string(labels, CONTAINER_PORTS)) {
    auto const ports = nlohmann::json::parse(*ports_string);
    if (ports.type() == nlohmann::json::value_t::array) {
      for (auto const &port : ports) {
        if (port.type() != nlohmann::json::value_t::object) {
          continue;
        }

        auto const name = try_get_string(port, CONTAINER_PORT_NAME);
        auto const number = try_get_int(port, CONTAINER_PORT_NUMBER);
        auto const protocol = try_get_string(port, CONTAINER_PORT_PROTOCOL);

        if (name && number && protocol) {
          auto const port_number = static_cast<std::uint16_t>(*number);
          ports_[port_number] = PortInfo{
              .port = port_number,
              .protocol = try_enum_from_string(*protocol, PortProtocol::unknown),
              .name = *name,
          };
        }
      }
    }
  }
}

K8sMetadata::operator bool() const
{
  return !container_name_.empty() || !pod_name_.empty() || !pod_ns_.empty() || !pod_uid_.empty() || !sandbox_uid_.empty();
}
