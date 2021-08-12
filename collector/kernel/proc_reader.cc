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

#include <collector/kernel/proc_reader.h>
#include <cstdio>
#include <stdexcept>

ProcReader::ProcReader() : pid_(0)
{
  proc_ = opendir("/proc");
  if (proc_ == NULL)
    throw std::runtime_error("ProcReader: Couldn't open proc.");
}

ProcReader::~ProcReader()
{
  closedir(proc_);
}

int ProcReader::get_pid()
{
  return pid_;
}

int ProcReader::is_pid()
{
  if (sscanf(proc_ent_->d_name, "%d", &pid_) != 1)
    return 0; // exit if the current proc_ent_ isn't a pid directory
  return 1;
}

int ProcReader::next()
{
  proc_ent_ = readdir(proc_);
  if (proc_ent_ == 0)
    return 0;
  return 1;
}
