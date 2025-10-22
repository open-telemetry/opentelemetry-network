// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/cgroup_handler.h>

#include <collector/agent_log.h>
#include <collector/constants.h>
#include <common/constants.h>
#include <config.h>
#include <util/docker_host_config_metadata.h>
#include <util/file_ops.h>
#include <util/k8s_metadata.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/nomad_metadata.h>

#include <generated/ebpf_net/agent_internal/wire_message.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline std::string make_docker_query_url(std::string const &container_name)
{
  static const std::string docker_query_base = "http://localhost/containers/";
  return docker_query_base + container_name + "/json";
}

std::string CgroupHandler::docker_ns_label_field;

CgroupHandler::CgroupHandler(
    ::ebpf_net::ingest::Writer &writer, CurlEngine &curl_engine, CgroupSettings const &settings, logging::Logger &log)
    : writer_(writer), curl_engine_(curl_engine), settings_(settings), log_(log)
{}

CgroupHandler::~CgroupHandler()
{
  // cancel all running queries
  for (auto &entry : queries_) {
    DockerQuery &query = entry.second;
    curl_engine_.cancel_fetch(*query.request);
  }
}

bool CgroupHandler::has_cgroup(u64 cgroup)
{
  return (cgroup_table_.find(cgroup) != cgroup_table_.end());
}

std::string_view CgroupHandler::get_name(u64 cgroup)
{
  auto pos = cgroup_table_.find(cgroup);
  if (pos == cgroup_table_.end()) {
    return {};
  }
  return pos->second.name;
}

/* END */
void CgroupHandler::kill_css(u64 timestamp, struct jb_agent_internal__kill_css *msg)
{
  std::string name{(char *)msg->name, strnlen((char *)msg->name, sizeof(msg->name))};

  LOG::debug_in(
      AgentLogKind::CGROUPS,
      "CgroupHandler::kill_css"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tcgroup_parent: 0x{:x}"
      "\n\tname: "
      "\n}}",
      msg->cgroup,
      msg->cgroup_parent,
      name);

  if (has_cgroup(msg->cgroup)) {
    LOG::debug_in(AgentLogKind::CGROUPS, "Success: cgroup found. \t{}", get_name(msg->cgroup));
  } else {
    log_.warn("kill_css(): cgroup not found: 0x{:x}", msg->cgroup);
  }

  auto pos = cgroup_table_.find(msg->cgroup);
  if (pos == cgroup_table_.end()) {
    return;
  }

  cgroup_table_.erase(pos);
}

/* START */
void CgroupHandler::css_populate_dir(u64 timestamp, struct jb_agent_internal__css_populate_dir *msg)
{
  std::string name{(char *)msg->name, strnlen((char *)msg->name, sizeof(msg->name))};

  LOG::debug_in(
      AgentLogKind::CGROUPS,
      "CgroupHandler::css_populate_dir"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tcgroup_parent: 0x{:x}"
      "\n\tname: {}"
      "\n}}",
      msg->cgroup,
      msg->cgroup_parent,
      name);

  if (has_cgroup(msg->cgroup_parent)) {
    LOG::debug_in(AgentLogKind::CGROUPS, "Success: cgroup->parent found. \t{}", get_name(msg->cgroup_parent));
  } else {
    log_.warn("cgroup->parent not found: cgroup_parent={:x}, cgroup={:x}, name={}", msg->cgroup_parent, msg->cgroup, name);
  }

  handle_cgroup(msg->cgroup, msg->cgroup_parent, name);
}

/* EXISTING */
void CgroupHandler::existing_cgroup_probe(u64 timestamp, struct jb_agent_internal__existing_cgroup_probe *msg)
{
  std::string name{(char *)msg->name, strnlen((char *)msg->name, sizeof(msg->name))};

  // msg->cgroup_parent, msg->name);
  LOG::debug_in(
      AgentLogKind::CGROUPS,
      "CgroupHandler::existing_cgroup_probe"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tcgroup_parent: 0x{:x}"
      "\n\tname: {}"
      "\n}}",
      msg->cgroup,
      msg->cgroup_parent,
      name);

  handle_cgroup(msg->cgroup, msg->cgroup_parent, name);
}

void CgroupHandler::cgroup_attach_task(u64 timestamp, struct jb_agent_internal__cgroup_attach_task *msg)
{
  LOG::debug_in(
      AgentLogKind::CGROUPS,
      "CgroupHandler::cgroup_attach_task"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tpid: {}"
      "\n\tcomm: {}"
      "\n}}",
      msg->cgroup,
      msg->pid,
      std::string_view((char *)msg->comm, strnlen((char *)msg->comm, sizeof(msg->comm))));

  if (has_cgroup(msg->cgroup)) {
    LOG::debug_in(AgentLogKind::CGROUPS, "Success: cgroup found. \t{}", get_name(msg->cgroup));
  } else {
    log_.warn("cgroup_attach_task(): cgroup not found: 0x{:x}", msg->cgroup);
  }
}

void CgroupHandler::handle_pid_info(u32 pid, u64 cgroup, uint8_t comm[16])
{
  LOG::debug_in(
      AgentLogKind::CGROUPS,
      "CgroupHandler::handle_pid_info"
      "\n{{"
      "\n\tpid: {}"
      "\n\tcgroup: 0x{:x}"
      "\n\tcomm: {}"
      "\n}}",
      pid,
      cgroup,
      std::string_view((char *)comm, strnlen((char *)comm, 16)));

  if (has_cgroup(cgroup)) {
    LOG::debug_in(AgentLogKind::CGROUPS, "Success: cgroup found. \t{}", get_name(cgroup));
  } else {
    log_.warn("handle_pid_info(): cgroup not found: 0x{:x}", cgroup);
  }
}

void CgroupHandler::handle_cgroup(u64 cgroup, u64 cgroup_parent, std::string const &name)
{
  auto emp = cgroup_table_.emplace(cgroup, CgroupEntry{cgroup_parent, name});
  if (emp.second == false) {
    return;
  }

  auto parent_pos = cgroup_table_.find(cgroup_parent);
  if (parent_pos == cgroup_table_.end()) {
    return;
  }

  bool is_docker = settings_.force_docker_metadata;

  if (!is_docker) {
    if (parent_pos->second.name == "docker") {
      // parent container's name is docker
      is_docker = true;
    } else {
      // grandparent
      auto gp_pos = cgroup_table_.find(parent_pos->second.cgroup_parent);
      if (gp_pos != cgroup_table_.end()) {
        if (gp_pos->second.name == "ecs") {
          // grandparent container's name is ecs
          is_docker = true;
        }
      }
    }
  }

  if (is_docker) {
    handle_docker_container(cgroup, name);
  }
}

void CgroupHandler::handle_docker_container(u64 cgroup, std::string const &name)
{
  LOG::debug_in(
      AgentLogKind::DOCKER,
      "CgroupHandler::handle_docker_container:"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tname: {}"
      "\n}}",
      cgroup,
      name);

  auto request = std::make_unique<CurlEngine::FetchRequest>(
      make_docker_query_url(name),
      [this, cgroup](const char *data, size_t data_length) { this->data_available_cb(data, data_length, cgroup); },
      [this, cgroup](CurlEngineStatus status, int responseCode, std::string_view curlError) {
        this->fetch_done_cb(status, responseCode, curlError, cgroup);
      });

  request->unix_socket(UNIX_SOCKET_PATH);

  // debug mode curl if debugging docker
  request->debug_mode(is_log_whitelisted(AgentLogKind::DOCKER));

  auto emp = queries_.emplace(cgroup, DockerQuery{std::move(request)});
  if (emp.second == false) {
    log_.error("query for cgroup:{} is already running", cgroup);
    return;
  }
  auto status = curl_engine_.schedule_fetch(*emp.first->second.request);
  if (status != CurlEngineStatus::OK) {
    // scheduling failure. curl engine will have called the done_fn, which
    // cleans up queries_
    log_.error("failed to schedule a fetch request");
    return;
  }

  LOG::debug_in(AgentLogKind::DOCKER, "\tqueries_.size(): {}", queries_.size());
}

void CgroupHandler::data_available_cb(const char *data, size_t data_length, u64 cgroup)
{
  std::string s(data, data_length);
  LOG::debug_in(
      AgentLogKind::DOCKER,
      "DataAvailableFn:"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tdata_length: {}"
      "\n\ts: {}"
      "\n}}",
      cgroup,
      data_length,
      s);

  auto pos = queries_.find(cgroup);
  if (pos == queries_.end()) {
    log_.error("query entry for cgroup: 0x{:x} not found", cgroup);
    return;
  }

  DockerQuery &query = pos->second;
  query.response.append(s);
}

void CgroupHandler::fetch_done_cb(CurlEngineStatus status, long responseCode, std::string_view curlError, u64 cgroup)
{
  bool success = (status == CurlEngineStatus::OK);

  LOG::debug_in(
      AgentLogKind::DOCKER,
      "FetchDoneFn:"
      "\n{{"
      "\n\tcgroup: 0x{:x}"
      "\n\tsuccess: {}"
      "\n}}",
      cgroup,
      success);

  auto pos = queries_.find(cgroup);
  if (pos == queries_.end()) {
    log_.error("query entry for cgroup: 0x{:x} not found", cgroup);
    return;
  }

  std::string response_data(std::move(pos->second.response));
  queries_.erase(pos);

  if (!success) {
    const std::string_view status_text = to_string(status);
    log_.error("docker fetch failed [{}:{}]: {}", status_text, responseCode, curlError);
    return;
  }

  if ((responseCode >= 200) && (responseCode <= 299)) {
    // success
    handle_docker_response(cgroup, response_data);
  } else if ((responseCode >= 500) && (responseCode <= 599)) {
    // server error
    log_.error("docker fetch failed with response {}", responseCode);
  }

  LOG::debug_in(AgentLogKind::DOCKER, "\tqueries_.size(): {}", queries_.size());
}

inline std::string get_string(json const &j)
{
  if (j.is_string()) {
    return j.get<std::string>();
  } else {
    return std::string();
  }
}

inline std::string get_string(json const &object, char const *key)
{
  if (!object.is_object()) {
    return std::string();
  }

  auto pos = object.find(key);
  if (pos != object.end()) {
    return get_string(*pos);
  } else {
    return std::string();
  }
}

inline jb_blob blob(std::string const &str)
{
  return jb_blob{str.c_str(), static_cast<u16>(str.size())};
}

void CgroupHandler::handle_docker_response(u64 cgroup, std::string const &response_data)
{
  std::string id;
  std::string name;
  std::string image;
  std::string ip_addr;
  std::string cluster;
  std::string container;
  std::string task_family;
  std::string task_version;
  std::string ns;
  std::string pod_name;

  std::optional<DockerHostConfigMetadata> docker_host_config;
  std::optional<NomadMetadata> nomad_metadata;
  std::optional<K8sMetadata> k8s_metadata;

  if (settings_.docker_metadata_dump_dir) {
    auto const dump_filename = fmt::format(
        "{}/docker-inspect.{}.{}.json",
        *settings_.docker_metadata_dump_dir,
        cgroup,
        std::chrono::system_clock::now().time_since_epoch().count());

    if (auto const error = write_file(dump_filename.c_str(), response_data)) {
      LOG::warn("failed to dump docker metadata to {}: {}", dump_filename, error);
    }
  }

  try {
    json root = json::parse(response_data);

    auto network = root["NetworkSettings"];
    auto config = root["Config"];
    auto host_config = root["HostConfig"];
    auto labels = config["Labels"];
    auto env = config["Env"];

    id = get_string(root, "Id");
    name = get_string(root, "Name");
    image = get_string(config, "Image");
    ip_addr = get_string(network, "IPAddress");

    cluster = get_string(labels, "com.amazonaws.ecs.cluster");

    container = get_string(labels, "com.amazonaws.ecs.container-name");
    if (container.empty()) {
      // Try k8s if ECS container is missing
      container = get_string(labels, "io.kubernetes.container.name");
    }

    pod_name = get_string(labels, "com.amazonaws.ecs.task-arn");
    if (pod_name.empty()) {
      // k8s
      pod_name = get_string(labels, "io.kubernetes.pod.name");
    }

    task_family = get_string(labels, "com.amazonaws.ecs.task-definition-family");
    task_version = get_string(labels, "com.amazonaws.ecs.task-definition-version");

    if (!docker_ns_label_field.empty()) {
      ns = get_string(labels, docker_ns_label_field.c_str());
    }

    docker_host_config.emplace(host_config);
    nomad_metadata.emplace(NomadMetadata(env));
    k8s_metadata.emplace(labels);

    for (auto const &item : labels.items()) {
      if (!item.value().is_string()) {
        continue;
      }

      std::string_view key = item.key();
      std::string const value = item.value().get<std::string>();

      if ((key.size() + value.size() + sizeof(u64) + jb_ingest__container_annotation__data_size) > WRITE_BUFFER_SIZE) {
        // NOTE: we use a substring of the key to make sure it fits in the
        // warning message.
        log_.warn("Docker metadata label for key '{}' is too large to send", key.substr(0, 256));
        continue;
      }

      writer_.container_annotation(cgroup, jb_blob{key}, jb_blob{value});
    }
  } catch (json::exception &e) {
    log_.error("failed to parse response data: {}", e.what());
    return;
  }

  if (docker_host_config.has_value()) {
    LOG::debug_in(AgentLogKind::DOCKER, "container resource limits: {}", fmt::streamed(*docker_host_config));

    auto const cpu_period = docker_host_config->cpu_period();
    auto const cpu_quota = docker_host_config->cpu_quota();
    writer_.container_resource_limits(
        cgroup,
        std::max(kernel::MIN_CGROUP_CPU_SHARES, std::min(kernel::MAX_CGROUP_CPU_SHARES, docker_host_config->cpu_shares())),
        cpu_period <= 0 ? kernel::DEFAULT_CGROUP_QUOTA : cpu_period,
        cpu_quota < 0 ? kernel::DEFAULT_CGROUP_QUOTA : cpu_quota,
        docker_host_config->memory_swappiness(),
        docker_host_config->memory_limit(),
        docker_host_config->memory_soft_limit(),
        docker_host_config->total_memory_limit());
  }

  LOG::debug_in(
      AgentLogKind::DOCKER,
      "container metadata:"
      "\n{{"
      "\n\tid: {}"
      "\n\tname: {}"
      "\n\timage: {}"
      "\n\tip_addr: {}"
      "\n\tcluster: {}"
      "\n\tcontainer: {}"
      "\n\ttask_family: {}"
      "\n\ttask_version: {}"
      "\n\tns: {}"
      "\n}}",
      id,
      name,
      image,
      ip_addr,
      cluster,
      container,
      task_family,
      task_version,
      ns);

  writer_.container_metadata(
      cgroup,
      blob(id),
      blob(name),
      blob(image),
      blob(ip_addr),
      blob(cluster),
      blob(container),
      blob(task_family),
      blob(task_version),
      blob(ns));

  if (nomad_metadata.has_value() && *nomad_metadata) {
    writer_.nomad_metadata(
        cgroup,
        jb_blob{nomad_metadata->ns()},
        jb_blob{nomad_metadata->group_name()},
        jb_blob{nomad_metadata->task_name()},
        jb_blob{nomad_metadata->job_name()});
  }

  if (!pod_name.empty()) {
    LOG::debug_in(
        AgentLogKind::DOCKER,
        "pod_name:"
        "\n{{"
        "\n\tname: {}"
        "\n}}",
        pod_name);

    writer_.pod_name(cgroup, blob("") /* deprecated pod_uid */, blob(pod_name));
  }

  if (k8s_metadata.has_value() && *k8s_metadata) {
    writer_.k8s_metadata(
        cgroup,
        jb_blob{k8s_metadata->container_name()},
        jb_blob{k8s_metadata->pod_name()},
        jb_blob{k8s_metadata->pod_ns()},
        jb_blob{k8s_metadata->pod_uid()},
        jb_blob{k8s_metadata->sandbox_uid()});

    for (auto const &port : k8s_metadata->ports()) {
      writer_.k8s_metadata_port(cgroup, port.second.port, integer_value(port.second.protocol), jb_blob{port.second.name});
    }
  };

  writer_.flush();
}
