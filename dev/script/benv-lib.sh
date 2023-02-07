#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

get_benv_container_name() {
  # MacOS has a slightly different syntax for format in stat
  if [ "$(uname -s)" == "Darwin" ]; then
    echo "benv-$(basename "${EBPF_NET_SRC}")-$(stat -f %i "${EBPF_NET_SRC}")"
  else
    echo "benv-$(basename "${EBPF_NET_SRC}")-$(stat --format=%i "${EBPF_NET_SRC}")"
  fi
}

get_benv_build_dir() {
  container_name=$(get_benv_container_name)
  benv_build_dir=""
  benv_build_dir=$(docker inspect "${container_name}" | jq .[].Mounts | grep tmp | awk '{print $2}' | sed 's/,$//' | sed 's/"//g') || true

  if [ -z "${benv_build_dir}" ]
  then
    echo "unable to find benv build directory for container name ${container_name}" 1>&2
    # Note: don't return with an error status to allow scripts to check for empty response and continue running
  fi

  echo "${benv_build_dir}"
}

