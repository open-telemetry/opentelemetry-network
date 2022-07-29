/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fstream>
#include <string>

/**
 * Reads through "/proc/net/tcp" and "/proc/net/tcp6"
 */
class ProcNetReader {
public:
  /**
   * c'tor
   */
  ProcNetReader(std::string filename);

  /**
   * d'tor
   */
  ~ProcNetReader();

  /**
   * Accessors for ino_, sk_, and sk_state_;
   */
  int get_ino();

  unsigned long get_sk();

  int get_sk_state();

  //
  // Read /proc//* for the next entry.
  // Returns 1 if a process entry was found and 0 otherwise.
  //
  int next();

private:
  std::ifstream tcp_file_;
  int sk_ino_;
  unsigned long sk_p_;
  int sk_state_;
};
