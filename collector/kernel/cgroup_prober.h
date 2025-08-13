/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <common/host_info.h>
#include <util/logger.h>

#include <linux/bpf.h>

#include <functional>
#include <string>

/* forward declarations */
class ProbeHandler;
struct render_bpf_bpf;

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
      struct render_bpf_bpf *skel,
      HostInfo const &host_info,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb);

  int error_count() { return close_dir_error_count_; };

private:
  /**
   * Locates the cgroup v1 directory that should be used for probing.
   */
  static std::string find_cgroup_v1_mountpoint();

  /**
   * Locates the cgroup v2 directory that should be used for probing.
   */
  static std::string find_cgroup_v2_mountpoint();

  /**
   * Recursively walks through directory structure and triggers the
   * corresponding existing croup probe by reading the file_name specified.
   *
   * @param cgroup_dir_name: path to directory in which to perform the search
   * @param file_name: file name to read
   * @param periodic_cb: callback to call after doing some work.
   */
  void trigger_existing_cgroup_probe(
      std::string const &cgroup_dir_name, std::string const &file_name, std::function<void(void)> periodic_cb);

  /**
   * Recursively walks through directory structure and triggers the
   * cgroup_get_from_fd probe by calling BPF_PROG_QUERY on each cgroup directory.
   *
   * @param cgroup_dir_name: path to directory in which to perform the search
   * @param periodic_cb: callback to call after doing some work.
   */
  void trigger_cgroup_get_from_fd_probe(std::string const &cgroup_dir_name, std::function<void(void)> periodic_cb);

  HostInfo const host_info_;
  int close_dir_error_count_;
};
