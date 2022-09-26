/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string>
#include <string_view>
#include <iostream>

// information extracted from the cgroup, used by CGroupParser (below).
struct CGroupInfo
{
  std::string container_id;
  std::string runtime;
  std::string pod_id;
  std::string qos;
  std::string service;

  // that is, if parsing failed
  bool valid = false;
};

// This class parses a cgroup name, attempting to extract information that
// is later useful: the container id, pod id, service, etc.
// It supports both cgroupfs and systemd cgroups.
//
// To use, pass the cgroup's name through the constructor, and obtain the
// results through the get() method.
class CGroupParser
{
public:
  explicit CGroupParser(std::string_view cgroup_name);
  
  const CGroupInfo& get() const { return info_; }
  std::string_view cgroup_name() const { return cgroup_name_; }

private:
  bool parse_cgroup();
  bool parse_systemd();
  bool parse_cri();
  bool parse_pod_id();
  bool parse_container_id();
  bool parse_qos();
  bool parse_uid();
  bool parse_uid_group(size_t count);
  bool parse_runtime(char token);
  bool parse_service();

  CGroupInfo info_;
  std::string_view cgroup_name_;
  typename std::string_view::const_iterator read_head_;
};
