// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/proc_net_reader.h>
#include <sstream>

ProcNetReader::ProcNetReader(std::string filename) : tcp_file_(filename), sk_ino_(0), sk_p_(0), sk_state_(0)
{
  // skip the first line of the file
  std::string line;
  getline(tcp_file_, line);
}

ProcNetReader::~ProcNetReader()
{
  tcp_file_.close();
}

int ProcNetReader::get_ino()
{
  return sk_ino_;
}

unsigned long ProcNetReader::get_sk()
{
  return sk_p_;
}

int ProcNetReader::get_sk_state()
{
  return sk_state_;
}

int ProcNetReader::next()
{
  std::string line;
  getline(tcp_file_, line);
  if (line == "")
    return 0;
  std::istringstream issline(line);
  std::string tk;
  int tk_id = 0;
  do {
    issline >> tk;
    tk_id++;
    if (tk_id == 4) { // parse the state
      sscanf(tk.c_str(), "%x", &sk_state_);
    } else if (tk_id == 10) { // parse inode_number
      sscanf(tk.c_str(), "%d", &sk_ino_);
    } else if (tk_id == 12) { // parse sk pointer
      sscanf(tk.c_str(), "%lx", &sk_p_);
    } else if (tk_id > 12) { // done
      break;
    }
  } while (issline);
  return 1;
}
