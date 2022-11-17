// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "cgroup_span.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <reducer/constants.h>
#include <reducer/uid_key.h>
#include <reducer/util/docker_image.h>

#include <common/constants.h>
#include <common/port_protocol.h>

#include <util/k8s_metadata.h>
#include <util/log.h>
#include <util/string_view.h>

#include <generated/ebpf_net/ingest/containers.inl>
#include <generated/ebpf_net/ingest/modifiers.h>
#include <generated/ebpf_net/ingest/weak_refs.inl>

#include <cstring>

namespace reducer::ingest {

namespace {

constexpr std::string_view UNKNOWN_JOB = "<unknown>";
constexpr std::string_view UNKNOWN_GROUP = "<unknown>";

constexpr std::string_view SYSTEMD_ROOT_CGROUP = "system.slice";
constexpr std::string_view SYSTEMD_SERVICE_SUFFIX = ".service";

} // namespace

CgroupSpan::CgroupSpan() {}

CgroupSpan::~CgroupSpan() {}

void CgroupSpan::cgroup_create_deprecated(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_create_deprecated *msg)
{
  auto create_message = jsrv_ingest__cgroup_create{};
  std::memcpy(create_message.name, msg->name, sizeof(msg->name));
  create_message.cgroup = msg->cgroup;
  create_message.cgroup_parent = msg->cgroup_parent;

  // deprecated message has 64 bytes for name (instead of 256), otherwise the message is the same.
  cgroup_create(span_ref, timestamp, &create_message);
}

void CgroupSpan::cgroup_create(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_create *msg)
{
  std::string_view name{(char const *)msg->name, strnlen((char const *)msg->name, sizeof(msg->name))};

  LOG::trace_in(
      Component::cgroup, "CgroupSpan::cgroup_create cgroup={} cgroup_parent={} name={}", msg->cgroup, msg->cgroup_parent, name);

  // attempt to extract info from the cgroup.
  CGroupParser parser{name};

  set_pod(span_ref, parser);
  set_parent(span_ref, msg->cgroup_parent);
  set_service(span_ref, parser);
  set_container(span_ref, parser);
}

void CgroupSpan::set_pod(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser)
{
  static constexpr size_t name_max_len = ::ebpf_net::ingest::modifiers::cgroup::name_t::max_len;
  // See CGroupParser.  The pod id may be embedded somewhere in the name.
  auto name = parser.cgroup_name();
  auto info = parser.get();

  // NOTE: if pod_uid is left empty, this will still do the right thing
  u64 pod_uid_hash = uid_to_u64(info.pod_id);
  std::array<u8, 64> pod_uid_suffix;
  uid_suffix(info.pod_id, pod_uid_suffix.data(), pod_uid_suffix.max_size());

  span_ref.modify()
      .name({name.data(), std::min(name.size(), name_max_len)})
      .pod_uid_hash(pod_uid_hash)
      .pod_uid_suffix(pod_uid_suffix)
      .cpu_soft_limit(kernel::MAX_CGROUP_CPU_SHARES)
      .cpu_hard_quota(kernel::DEFAULT_CGROUP_QUOTA)
      .cpu_hard_period(kernel::DEFAULT_CGROUP_QUOTA);
}

void CgroupSpan::set_parent(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 cgroup_parent)
{
  auto *conn = local_connection()->ingest_connection();

  if (auto parent_ref = conn->get_cgroup(cgroup_parent); parent_ref.valid()) {
    span_ref.modify().parent(parent_ref.get());
  } else {
    LOG::trace_in(Component::cgroup, "CgroupSpan: unable to find parent cgroup");
  }
}

void CgroupSpan::set_container(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser)
{
  static constexpr size_t id_max_len = ::ebpf_net::ingest::modifiers::container::id_t::max_len;

  auto name = parser.cgroup_name();
  auto info = parser.get();

  // mostly following the previous behavior, where it was always assigning the cgroup
  // name as the container_id.  However, we do try and extract the container id if
  // possible (for cgroups where it is embedded somewhere in the name).
  std::string_view container_id = info.container_id.empty() ? name : info.container_id;

  assert(!span_ref.container().valid());

  ::ebpf_net::ingest::keys::container container_key;
  container_key.id.set(container_id.data(), std::min(container_id.size(), id_max_len));

  auto container = local_index()->container.by_key(container_key);
  if (container.valid()) {
    span_ref.modify().container(container.get());
  } else {
    LOG::trace_in(Component::cgroup, "CgroupSpan: could not allocate a container span");
  }
}

void CgroupSpan::set_service(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser)
{
  auto parent = span_ref.parent();

  if (!parent.valid()) {
    return;
  }

  if (parent.service().valid()) {
    // Inherit parent's service.
    span_ref.modify().service(parent.service().get());
    return;
  }

  // cgroups where the name ends in ".service" (see CGroupParser::parse_service())
  auto info = parser.get();
  if ((parent.name() == SYSTEMD_ROOT_CGROUP) && !info.service.empty()) {
    auto service = local_index()->service.by_key({{short_string_behavior::truncate, info.service}});

    if (!service.valid()) {
      LOG::error("CgroupSpan: could not allocate a service span");
      return;
    }

    span_ref.modify().service(std::move(service));
  }
}

void CgroupSpan::cgroup_close(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_close *msg)
{
  LOG::trace_in(Component::cgroup, "CgroupSpan::cgroup_close cgroup:{}", msg->cgroup);
}

void CgroupSpan::container_metadata(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_metadata *msg)
{
  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::container_metadata cgroup:{} container_name: {}",
      msg->cgroup,
      msg->container_name.string_view());

  using role_t = ::ebpf_net::ingest::spans::container::role_t;
  using version_t = ::ebpf_net::ingest::spans::container::version_t;
  using ns_t = ::ebpf_net::ingest::spans::container::ns_t;
  using name_t = ::ebpf_net::ingest::spans::container::name_t;

  auto container = span_ref.container();
  if (!container.valid()) {
    return;
  }

  auto const container_name = msg->container_name.string_view();
  container.modify().name({container_name.data(), std::min(container_name.size(), name_t::max_len)});

  std::string_view role;
  std::string_view version;

  if (msg->task_family.len > 0) {
    auto const task_family = msg->task_family.string_view();
    auto const task_version = msg->task_version.string_view();

    LOG::trace_in(Component::cgroup, "CgroupSpan: using task family='{}', version='{}' for role", task_family, task_version);

    role = task_family;
    version = task_version;
  } else if (!k8s_container_name_.empty() && k8s_container_name_ != K8sMetadata::POD_CONTAINER_NAME_VALUE) {
    role = k8s_container_name_;

    DockerImageMetadata const image{msg->image.string_view()};
    version = image.version();
  } else if (auto name = msg->name.string_view(); !name.empty()) {
    DockerImageMetadata const image{msg->image.string_view()};

    LOG::trace_in(Component::cgroup, "CgroupSpan: using name='{}', image='{}' for role", name, msg->image);

    if (name.front() == '/') {
      // strip the leading slash
      name.remove_prefix(1);
    }

    role = name;

    version = image.version();
  }

  auto const ns = msg->ns.string_view();

  container.modify()
      .role({role.data(), std::min(role.size(), role_t::max_len)})
      .version({version.data(), std::min(version.size(), version_t::max_len)})
      .ns({ns.data(), std::min(ns.size(), ns_t::max_len)})
      .node_type(integer_value(NodeResolutionType::CONTAINER));

  on_container_updated(container);

  if (!role.empty()) {
    LOG::trace_in(Component::cgroup, "CgroupSpan: role='{}', version='{}', ns='{}'", role, version, ns);
  }
}

void CgroupSpan::container_annotation(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_annotation *msg)
{
  if (msg->key == K8sMetadata::CONTAINER_NAME) {
    k8s_container_name_ = msg->value.to_string();
  } else {
    annotations_[msg->key.to_string()] = msg->value.to_string();
  }
}

void CgroupSpan::pod_name(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__pod_name *msg)
{
  auto container = span_ref.container();
  if (!container.valid()) {
    return;
  }

  container.modify().pod_name({short_string_behavior::truncate, msg->name});

  on_container_updated(container);
}

void CgroupSpan::k8s_metadata(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__k8s_metadata *msg)
{
  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::k8s_metadata(cgroup=`{}` container_name=`{}` pod_name=`{}` pod_ns=`{}`"
      " pod_uid=`{}` sandbox_uid=`{}`)",
      msg->cgroup,
      msg->container_name,
      msg->pod_name,
      msg->pod_ns,
      msg->pod_uid,
      msg->sandbox_uid);

  auto container = span_ref.container();
  if (!container.valid()) {
    return;
  }

  container.modify()
      .ns({short_string_behavior::truncate, msg->pod_ns})
      .pod_name({short_string_behavior::truncate, msg->pod_name});

  on_container_updated(container);
}

void CgroupSpan::k8s_metadata_port(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__k8s_metadata_port *msg)
{
  auto const protocol = sanitize_enum(static_cast<PortProtocol>(msg->protocol));

  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::k8s_metadata_port(cgroup=`{}` port=`{}` protocol=`{}` name=`{}`)",
      msg->cgroup,
      msg->port,
      protocol,
      msg->name);

  // TODO: consume port mappings
}

void CgroupSpan::nomad_metadata(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__nomad_metadata *msg)
{
  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::nomad_metadata(cgroup=`{}` ns=`{}` group=`{}` task=`{}` job=`{}`)",
      msg->cgroup,
      msg->ns,
      msg->group_name,
      msg->task_name,
      msg->job_name);

  auto container = span_ref.container();
  if (!container.valid()) {
    return;
  }

  bool updated = false;

  if (auto const ns = msg->ns.string_view(); !ns.empty()) {
    using ns_t = ::ebpf_net::ingest::spans::container::ns_t;
    container.modify().ns({ns.data(), std::min(ns.size(), ns_t::max_len)});
    updated = true;
  }

  if (auto const task_name = msg->task_name.string_view(); !task_name.empty()) {
    auto const job_name = msg->job_name.string_view();
    auto const group_name = msg->group_name.string_view();
    auto const role = fmt::format(
        "{}.{}.{}", job_name.empty() ? UNKNOWN_JOB : job_name, group_name.empty() ? UNKNOWN_GROUP : group_name, task_name);

    using role_t = ::ebpf_net::ingest::spans::container::role_t;
    container.modify().role({role.data(), std::min(role.size(), role_t::max_len)});
    updated = true;
  }

  if (updated) {
    container.modify().node_type(integer_value(NodeResolutionType::NOMAD));
    on_container_updated(container);
  }
}

void CgroupSpan::container_resource_limits_deprecated(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_resource_limits_deprecated *msg)
{
  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::container_resource_limits_deprecated: cgroup=`{}`"
      " port=`{}` protocol=`{}` name=`{}`",
      " cpu_shares=`{}` cpu_period=`{}` cpu_quota=`{}`"
      " memory_swappiness=`{}` memory_limit=`{}` memory_soft_limit=`{}` total_memory_limit=`{}`",
      msg->cgroup,
      msg->cpu_shares,
      msg->cpu_period,
      msg->cpu_quota,
      msg->memory_swappiness,
      msg->memory_limit,
      msg->memory_soft_limit,
      msg->total_memory_limit);

  auto const period = msg->cpu_period ? msg->cpu_period : kernel::DEFAULT_CGROUP_QUOTA;
  auto const quota = msg->cpu_quota ? msg->cpu_quota : period;

  span_ref.modify().cpu_soft_limit(msg->cpu_shares).cpu_hard_quota(quota).cpu_hard_period(period);
}

void CgroupSpan::container_resource_limits(
    ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_resource_limits *msg)
{
  LOG::trace_in(
      Component::cgroup,
      "CgroupSpan::container_resource_limits: cgroup=`{}`"
      " port=`{}` protocol=`{}` name=`{}`",
      " cpu_shares=`{}` cpu_period=`{}` cpu_quota=`{}`"
      " memory_swappiness=`{}` memory_limit=`{}` memory_soft_limit=`{}` total_memory_limit=`{}`",
      msg->cgroup,
      msg->cpu_shares,
      msg->cpu_period,
      msg->cpu_quota,
      msg->memory_swappiness,
      msg->memory_limit,
      msg->memory_soft_limit,
      msg->total_memory_limit);

  auto const period = msg->cpu_period ? msg->cpu_period : kernel::DEFAULT_CGROUP_QUOTA;
  auto const quota = msg->cpu_quota ? msg->cpu_quota : period;

  span_ref.modify().cpu_soft_limit(msg->cpu_shares).cpu_hard_quota(quota).cpu_hard_period(period);
}

void CgroupSpan::on_container_updated(::ebpf_net::ingest::weak_refs::container container_ref)
{
  // Increase the update counter to signal that the container metadata has
  // changed and needs to be sent to the matching core.
  container_ref.modify().update_count(container_ref.update_count() + 1);
}

} // namespace reducer::ingest
