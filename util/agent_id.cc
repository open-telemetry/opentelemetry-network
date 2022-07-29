// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
