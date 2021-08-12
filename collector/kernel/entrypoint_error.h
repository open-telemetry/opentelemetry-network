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

#define ENUM_NAME EntrypointError
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(none, 1)                                                                                                                   \
  X(unsupported_distro, 2)                                                                                                     \
  X(kernel_headers_fetch_error, 3)                                                                                             \
  X(kernel_headers_fetch_refuse, 4)                                                                                            \
  X(kernel_headers_missing_repo, 5)                                                                                            \
  X(kernel_headers_misconfigured_repo, 6)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
