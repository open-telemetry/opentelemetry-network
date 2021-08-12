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

#pragma once

#include <util/enum.h>

#define ENUM_NAME AgentLogKind
#define ENUM_TYPE uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(BPF, 0)                                                                                                                    \
  X(UDP, 1)                                                                                                                    \
  X(DNS, 2)                                                                                                                    \
  X(TCP, 3)                                                                                                                    \
  X(HTTP, 4)                                                                                                                   \
  X(NAT, 6)                                                                                                                    \
  X(DOCKER, 7)                                                                                                                 \
  X(FLOW, 8)                                                                                                                   \
  X(CGROUPS, 9)                                                                                                                \
  X(PERF, 10)                                                                                                                  \
  X(PID, 11)                                                                                                                   \
  X(PROTOCOL, 12)                                                                                                              \
  X(CPU_MEM_IO, 13)                                                                                                            \
  X(NOMAD, 14)                                                                                                                 \
  X(SOCKET, 15)
#define ENUM_DEFAULT BPF
#include <util/enum_operators.inl>
