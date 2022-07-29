#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


# shellcheck disable=SC1091
[[ ! -e ./debug-info.conf ]] || source ./debug-info.conf

# to run the collector under gdb, set `EBPF_NET_RUN_UNDER_GDB` to the flavor of gdb
# you want (e.g.: `cgdb` or `gdb`) - this is intended for development purposes
if [[ -n "${EBPF_NET_RUN_UNDER_GDB}" ]]; then
  apt-get update -y
  apt-get install -y --no-install-recommends "${EBPF_NET_RUN_UNDER_GDB}"

  if [[ "${#EBPF_NET_GDB_COMMANDS[@]}" -lt 1 ]]; then
    # default behavior is to run the agent, print a stack trace after it exits
    # and exit gdb without confirmation
    EBPF_NET_GDB_COMMANDS=( \
      'set pagination off'
      'handle SIGPIPE nostop pass'
      'handle SIGUSR1 nostop pass'
      'handle SIGUSR2 nostop pass'
      run
      bt
      'server q'
    )
  fi

  GDB_ARGS=()
  for gdb_cmd in "${EBPF_NET_GDB_COMMANDS[@]}"; do
    GDB_ARGS+=(-ex "${gdb_cmd}")
  done

  (set -x; exec "${EBPF_NET_RUN_UNDER_GDB}" -q "${GDB_ARGS[@]}" \
    --args /srv/k8s-relay "$@" \
  )
else
  (set -x; exec /srv/k8s-relay "$@")
fi
