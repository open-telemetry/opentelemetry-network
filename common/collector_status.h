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

#define ENUM_NAME CollectorStatus
#define ENUM_NAMESPACE collector
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(healthy, 0)                                                                                                                \
  X(unknown, 1)                                                                                                                \
  X(aws_describe_regions_error, 2)                                                                                             \
  X(aws_describe_network_interfaces_error, 3)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
