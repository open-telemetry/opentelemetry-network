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
