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

#include <bcc/BPF.h>
#include <linux/bpf.h>

#include <functional>
#include <memory>

#include <platform/types.h>

#include <util/logger.h>

/* forward declarations */
class ProbeHandler;

/**
 * Adds BPF probes for new and existing sockets, and iterates through existing
 *   sockets, to obtain an up-to-date view of system sockets
 */
class SocketProber {
public:
  /**
   * C'tor
   *
   * @param probe_handler: a ProbeHandler where new probes can be registered
   * @param bpf_module: the module from the bpf source code
   * @param periodic_cb: a callback to be called every once in a while, to
   *   allow user to e.g., flush rings
   */
  SocketProber(
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb,
      logging::Logger &log);

private:
  /**
   * Fills the given map with a mapping of inode->pid of existing sockets
   *
   * @param map: the inode->pid map to fill
   * @param periodic_cb: callback to call after doing some work.
   */
  void fill_inode_to_pid_map(ebpf::BPFHashTable<u32, u32> &map, std::function<void(void)> periodic_cb);

  /**
   * Iterates through proc, and triggers the corresponding seq_show functions
   *   for all supported types of existing sockets by reading proc namespaces
   *
   * @param periodic_cb: callback to call after doing some work.
   */
  void trigger_seq_show(std::function<void(void)> periodic_cb);

  /**
   * Reads a file in /proc/<pid>/net/{tcp,tcp6}
   *
   * @param filename: the file to read
   * @param periodic_cb: callback to call after doing some work.
   */
  void read_proc_net_tcp(const std::string &filename, std::function<void(void)> periodic_cb);

  /**
   * Reads a file in /proc/<pid>/net/{udp,udp6}
   *
   * @param filename: the file to read
   * @param periodic_cb: callback to call after doing some work.
   */
  void read_proc_net_udp(const std::string &filename, std::function<void(void)> periodic_cb);

  /**
   * Returns the network namespace the pid lives in, by reading /proc
   *
   * @param pid: the pid to check
   * @returns namespace ID on success, -1 on failure.
   */
  int get_network_namespace(int pid);

private:
  logging::Logger &log_;
};
