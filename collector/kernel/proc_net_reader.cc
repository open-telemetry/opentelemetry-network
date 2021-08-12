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
