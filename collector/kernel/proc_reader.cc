// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
