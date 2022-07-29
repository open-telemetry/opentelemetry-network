/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dirent.h>
/**
 * Reads through proc
 */
class ProcReader {
public:
  /**
   * c'tor
   * throws if buff_ can't be malloc-ed
   */
  ProcReader();

  /**
   * d'tor
   */
  ~ProcReader();

  /**
   * Acts as an accessor to pid_
   * Assumes we always call is_pid() first.
   */
  int get_pid();

  /**
   * Returns 1 if the current proc_ent_ is a pid directory. Returns 0 otherwise
   */
  int is_pid();

  //
  // Reads /proc/* for the next entry.
  // Returns 1 if a process entry was found and 0 otherwise.
  //
  int next();

private:
  DIR *proc_;
  dirent *proc_ent_;
  int pid_;
};
