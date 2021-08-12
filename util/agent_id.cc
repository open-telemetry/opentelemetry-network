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

#include "agent_id.h"

#include <random>

std::string gen_agent_id()
{
  static const char chars[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
  const size_t idlen = 32;

  std::random_device rd;                                      // seed with non-deterministic random values from the system
  std::uniform_int_distribution<> dist(0, sizeof(chars) - 2); // range inclusive + zero terminator

  std::string id{"FAID"};

  for (size_t i = 0; i < idlen; i++) {
    id.push_back(chars[dist(rd)]);
  }

  return id;
}
