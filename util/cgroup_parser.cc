/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cgroup_parser.h"

#include "parser_utils.h"
#include "string_view.h"
#include "log.h"

#include <cctype>

using namespace parsing;

namespace {
  // just a little adapter for the c-style classification fn
  bool is_hex_digit(char c) {
    return isxdigit((int)c) > 0;
  }
}

CGroupParser::CGroupParser(std::string_view cgroup_name) :
  cgroup_name_(cgroup_name),
  read_head_(cgroup_name_.begin())
{
  // enter and descend
  info_.valid = parse_cgroup();
  LOG::trace(
      "CGroupParser info for"
      " cgroup_name: '{}'"
      " container_id: '{}' "
      " pod_id: '{}'"
      " qos: '{}'"
      " runtime: '{}'"
      " service: '{}'"
      " valid: '{}'",
      cgroup_name_,
      info_.container_id,
      info_.pod_id,
      info_.qos,
      info_.runtime,
      info_.service,
      info_.valid);
}

bool CGroupParser::parse_cgroup() {
  // the seen cgroup formats
  if (!parse_systemd() && 
      !parse_cri() && 
      !parse_pod_id() && 
      !parse_container_id() &&
      !parse_service()) {
    return false;
  }

  return true;
}

bool CGroupParser::parse_systemd()
{
  // systemd style cgroups.  note: uses '-' to seperate the parent-child
  // relations
  //
  // examples:
  // kubepods-burstable-pod146bb920_a47b_4f6c_a69a_166b63944d15.slice:cri-containerd:c45f3e9c19746eabf0a4af63d780ba5c2a657a7352c7ad7acc5d599da5115eef
  // kubepods-besteffort-pod29c71929_0064_4c15_9595_702c5931a368.slice
  if (!parse_match(read_head_, cgroup_name_.end(), "kubepods-")) {
    return false;
  }

  // quality of service classification
  if (!parse_qos()) {
    return false;
  }

  // eat until the next separator - we don't care about ".slice:..." if present
  parse_token(read_head_, cgroup_name_.end(), '-');

 // optional - may just be kubepods-<qos>.slice
  // pod unique id
  if (!parse_pod_id()) {
    // done and valid
    return true;
  }
  
  // eat until the next separator - we don't care about ".slice:..." if present
  parse_token(read_head_, cgroup_name_.end(), '-');

  // optional - may just be kubepods-<qos>-pod<pod_id>.slice
  // containerd and friends
  if (!parse_runtime(':')) {
    // done and valid
    return true;
  }

  // optional
  // the container's id
  if (!parse_container_id()) {
    return true;
  }

  return true;
}

bool CGroupParser::parse_cri() {
  // cgroups starting with 'cri'.  this has a runtime and 
  // a container id
  // example:
  // cri-containerd-15736ea91752be37a640dc949e3e805521f4af5c5e3fe50643af0e63a5ce0df5.scope
  if (!parse_match(read_head_, cgroup_name_.end(), "cri-")) {
    return false;
  }

  // containerd and friends
  if (!parse_runtime('-')) {
    return false;
  }

  // the container's id
  if (!parse_container_id()) {
    return false;
  }

  return true;
}

bool CGroupParser::parse_container_id() {
  // just 64 hex characters for the container's id
  // example:
  // 6f652f89943b50f7b101d13f11371daf34bf836b7e1b725b5e8b6439451018bd
  for (size_t ii = 0; ii < 64; ++ii) {
    if (!parse_match(read_head_, cgroup_name_.end(), is_hex_digit, &info_.container_id)) {
      info_.container_id.clear();
      return false;
    }
  }
  return true;
}

bool CGroupParser::parse_pod_id() {
  // "pod" followed by a unique pod id.
  // the format of the unique id varies between cgroupfs and systemd
  // we normalize to the canonical representation
  // examples:
  // podf55fb707-9bf6-4bf5-8a7e-19c5f3e52215
  // podf55fb707_9bf6_4bf5_8a7e_19c5f3e52215
  // podf55fb7079bf64bf58a7e19c5f3e52215
  if (!parse_match(read_head_, cgroup_name_.end(), "pod")) {
    return false;
  }

  return parse_uid();
}

bool CGroupParser::parse_service() {
  // service is ambiguous, so just peek at the suffix to see if we are a 
  // service
  //
  // example :
  // systemd-journald.service
  static constexpr std::string_view SERVICE_SUFFIX = ".service";
  if (views::ends_with(cgroup_name_, SERVICE_SUFFIX)) {
    info_.service = cgroup_name_.substr(0, cgroup_name_.size() - SERVICE_SUFFIX.size());
    return true;
  }
  return false;
}

bool CGroupParser::parse_qos() {
  // see https://kubernetes.io/docs/tasks/configure-pod-container/quality-service-pod/
  if (parse_match(read_head_, cgroup_name_.end(), "guaranteed", &info_.qos)) {
    return true;
  }

  if (parse_match(read_head_, cgroup_name_.end(), "besteffort", &info_.qos)) {
    return true;
  }

  if (parse_match(read_head_, cgroup_name_.end(), "burstable", &info_.qos)) {
    return true;
  }

  return false;
}

bool CGroupParser::parse_uid() {
  // parse the uuid groups, converting to the canonical format
  for(size_t ii : {8, 4, 4, 4, 12}) {
    if (!parse_uid_group(ii)) {
      return false;
    }

    // if it's not the last group, handle the differences in seen
    // formats.  it may have a '-' or '_' to separate, or 
    // no separator at all.
    if (ii != 12) {
      char c;
      if (!peek(read_head_, cgroup_name_.end(), &c)) {
        return false;
      }

      // the next char may be '_' (for systemd style), '-' (cgroupfs),
      // or no separator. regardless, convert to canonical style:
      // 8-4-4-4-12
      if (c == '-' || c == '_') {
        // eat the separator, if present
        consume(read_head_, cgroup_name_.end());
      }

      // append the canonical separator
      info_.pod_id += "-";
    }
  }

  return true;
}

bool CGroupParser::parse_uid_group(size_t count)
{
  // all the hex digits in the group (8,4, or 12)
  for(size_t ii = 0; ii < count; ++ii) {
    if (!parse_match(read_head_, cgroup_name_.end(), is_hex_digit, &info_.pod_id)) {
      return false;
    }
  }
  return true;
}

bool CGroupParser::parse_runtime(char token) {
  // ought we to validate this? containerd, etc...
  return parse_token(read_head_, cgroup_name_.end(), token, &info_.runtime);
}
