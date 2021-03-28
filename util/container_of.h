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

/**
 * C++-compatible container-of implementation.
 *
 * Inspired by https://stackoverflow.com/a/40851139
 */

#pragma once

#include <cstddef>

template <class P, class M> std::size_t fp_offsetof(const M P::*member)
{
  return (std::size_t) & (reinterpret_cast<P *>(0)->*member);
}

template <class P, class M> P *fp_container_of(M *ptr, const M P::*member)
{
  return (P *)((char *)ptr - fp_offsetof(member));
}
