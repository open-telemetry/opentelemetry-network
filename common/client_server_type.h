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

// This enum is used in BPF, Agent, and Server. Must be a normal enum because BPF/BCC is C, not C++.

// Server/client type enum
enum CLIENT_SERVER_TYPE {
  SC_CLIENT = 0,
  SC_SERVER = 1,
};

#ifndef _PROCESSING_BPF

inline const char *client_server_type_to_string(enum CLIENT_SERVER_TYPE client_server)
{
  switch (client_server) {
  case SC_CLIENT:
    return "CLIENT";
  case SC_SERVER:
    return "SERVER";
  default:
    break;
  }
  throw std::runtime_error("invalid CLIENT_SERVER_TYPE value");
}

#endif
