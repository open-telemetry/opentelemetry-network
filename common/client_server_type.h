/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
