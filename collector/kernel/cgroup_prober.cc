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

#include <collector/agent_log.h>
#include <collector/kernel/cgroup_prober.h>
#include <collector/kernel/fd_reader.h>
#include <collector/kernel/probe_handler.h>
#include <collector/kernel/proc_reader.h>

#include <fstream>
#include <iostream>
#include <set>
#include <stack>
#include <string>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

CgroupProber::CgroupProber(
    ProbeHandler &probe_handler,
    ebpf::BPFModule &bpf_module,
    std::function<void(void)> periodic_cb,
    std::function<void(std::string)> check_cb)
    : close_dir_error_count_(0)
{
  // END
  if (probe_handler.start_probe(bpf_module, "on_kill_css", "kill_css") != 0) {
    probe_handler.start_probe(bpf_module, "on_cgroup_destroy_locked", "cgroup_destroy_locked");
  }
  periodic_cb();

  // START
  if (probe_handler.start_probe(bpf_module, "on_css_populate_dir", "css_populate_dir") != 0) {
    probe_handler.start_probe(bpf_module, "on_cgroup_populate_dir", "cgroup_populate_dir");
  }
  periodic_cb();

  // EXISTING
  probe_handler.start_probe(bpf_module, "on_cgroup_clone_children_read", "cgroup_clone_children_read");
  probe_handler.start_probe(bpf_module, "on_cgroup_attach_task", "cgroup_attach_task");
  periodic_cb();
  check_cb("cgroup prober startup");

  // locate the cgroup mount directory
  std::string cgroup_mountpoint = find_cgroup_mountpoint();

  if (!cgroup_mountpoint.empty()) {
    // now iterate over cgroups and trigger cgroup_clone_children_read
    trigger_cgroup_clone_children_read(cgroup_mountpoint, periodic_cb);
    check_cb("trigger_cgroup_clone_children_read()");
  }

  /* can remove existing now */
  probe_handler.cleanup_probe("p_cgroup_clone_children_read");
  periodic_cb();
  check_cb("cgroup prober cleanup()");
}

void CgroupProber::trigger_cgroup_clone_children_read(std::string dir_name, std::function<void(void)> periodic_cb)
{
  std::stack<std::string> dirs_stack;
  dirs_stack.emplace(dir_name);
  while (!dirs_stack.empty()) {
    periodic_cb();
    // get the directory on the top of our stack
    std::string dir_name(dirs_stack.top());
    dirs_stack.pop();

    DIR *dir;
    dir = opendir(dir_name.c_str());
    if (!dir)
      continue;

    // trigger the probe on "cgroup_clone_children_read" for this directory
    std::string clone_children_path = dir_name + "/cgroup.clone_children";
    LOG::debug_in(AgentLogKind::CGROUPS, "cgroup_clone_children_read: path={}", clone_children_path);
    std::ifstream file(clone_children_path.c_str());
    if (file.fail()) {
      LOG::debug_in(AgentLogKind::CGROUPS, "   fail for path={}", clone_children_path);
      int status = closedir(dir);
      if (status != 0) {
        close_dir_error_count_++;
      }
      continue;
    } else {
      LOG::debug_in(AgentLogKind::CGROUPS, "   success for path={}", clone_children_path);
    }
    std::string line;
    std::getline(file, line);

    // iterate over the elements of this directory and add any
    // subdirectories to dirs_stack
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_DIR) {
        // skip over "." and ".." entries in the directory
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
          continue;

        dirs_stack.emplace(dir_name + "/" + ent->d_name);
      }
      periodic_cb();
    }
    int status = closedir(dir);
    if (status != 0) {
      close_dir_error_count_++;
    }
  }
}

static bool file_exists(std::string file_path)
{
  struct stat sb;

  if (stat(file_path.c_str(), &sb) == -1) {
    return false;
  }

  return S_ISREG(sb.st_mode);
}

static bool is_cgroup_mountpoint(std::string dir_path)
{
  static const std::string file_name("/cgroup.clone_children");

  return file_exists(dir_path + file_name);
}

std::string CgroupProber::find_cgroup_mountpoint()
{
  if (is_cgroup_mountpoint("/hostfs/sys/fs/cgroup/memory")) {
    return "/hostfs/sys/fs/cgroup/memory";
  }

  if (is_cgroup_mountpoint("/hostfs/cgroup/memory")) {
    return "/hostfs/cgroup/memory";
  }

  if (is_cgroup_mountpoint("/sys/fs/cgroup/memory")) {
    return "/sys/fs/cgroup/memory";
  }

  if (is_cgroup_mountpoint("/cgroup/memory")) {
    return "/cgroup/memory";
  }

  return std::string();
}
