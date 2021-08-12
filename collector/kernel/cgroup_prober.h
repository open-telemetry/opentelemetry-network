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

#include <linux/bpf.h>

#include <bcc/BPF.h>

#include <functional>
#include <string>
#include <util/logger.h>

/* forward declarations */
class ProbeHandler;

/**
 * Adds BPF probes for new and existing cgroups, and iterates through existing
 *   cgroups to obtain an up-to-date view of system cgroups
 */
class CgroupProber {
public:
  /**
   * C'tor
   *
   * @param probe_handler: a ProbeHandler where new probes can be registered
   * @param bpf_module: the module from the bpf source code
   * @param periodic_cb: a callback to be called every once in a while, to
   *   allow user to e.g., flush rings
   */
  CgroupProber(
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb);

  int error_count() { return close_dir_error_count_; };

private:
  /**
   * Locates the cgroup directory that should be used for probing.
   */
  static std::string find_cgroup_mountpoint();

  /**
   * Recursively walks through directory structure and triggers the
   * corresponding cgroup_clone_children_read functions by reading
   * cgroup.clone_children files.
   *
   * @param dir_name: path to directory in which to perform the search
   * @param periodic_cb: callback to call after doing some work.
   */
  void trigger_cgroup_clone_children_read(std::string dir_name, std::function<void(void)> periodic_cb);

  int close_dir_error_count_;
};
