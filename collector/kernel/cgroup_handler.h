/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/writer.h>
#include <util/curl_engine.h>
#include <util/logger.h>
#include <util/lookup3_hasher.h>

#include <optional>
#include <unordered_map>

static constexpr std::string_view UNIX_SOCKET_PATH = "/var/run/docker.sock";

class CgroupHandler {
public:
  struct CgroupSettings {
    bool force_docker_metadata = false;
    std::optional<std::string> docker_metadata_dump_dir;
  };

  // When set, specifies the name of the docker label that will be used for
  // obtaining the 'namespace' value.
  static std::string docker_ns_label_field;

  CgroupHandler(
      ::ebpf_net::ingest::Writer &writer, CurlEngine &curl_engine, CgroupSettings const &settings, logging::Logger &log);
  ~CgroupHandler();

  void kill_css(u64 timestamp, struct jb_agent_internal__kill_css *msg);
  void css_populate_dir(u64 timestamp, struct jb_agent_internal__css_populate_dir *msg);
  void cgroup_clone_children_read(u64 timestamp, struct jb_agent_internal__cgroup_clone_children_read *msg);
  void cgroup_attach_task(u64 timestamp, struct jb_agent_internal__cgroup_attach_task *msg);
  void handle_pid_info(u32 pid, u64 cgroup, uint8_t comm[16]);

private:
  friend class CgroupHandlerTest_handle_docker_response_Test;

  struct CgroupEntry {
    u64 cgroup_parent;
    std::string name;
  };

  struct DockerQuery {
    std::unique_ptr<CurlEngine::FetchRequest> request;
    std::string response;
  };

  ::ebpf_net::ingest::Writer &writer_;
  CurlEngine &curl_engine_;
  CgroupSettings const &settings_;
  logging::Logger &log_;
  std::unordered_map<u64, CgroupEntry> cgroup_table_;
  std::unordered_map<u64, DockerQuery> queries_;

  bool has_cgroup(u64 cgroup);
  // returns empty string for unknown cgroups
  std::string_view get_name(u64 cgroup);

  void handle_cgroup(u64 cgroup, u64 cgroup_parent, std::string const &name);
  void handle_docker_container(u64 cgroup, std::string const &name);

  void data_available_cb(const char *data, size_t data_length, u64 cgroup);
  void fetch_done_cb(CurlEngineStatus status, long responseCode, std::string_view curlError, u64 cgroup);

  void handle_docker_response(u64 cgroup, std::string const &response_data);
};
