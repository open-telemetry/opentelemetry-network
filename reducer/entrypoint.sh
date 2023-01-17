#!/bin/bash -x
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# shellcheck disable=SC1091
[[ ! -e ./debug-info.conf ]] || source ./debug-info.conf

install_dir=${EBPF_NET_INSTALL_DIR:-/srv}
reducer="${install_dir}/opentelemetry-ebpf-reducer"

data_dir=${EBPF_NET_DATA_DIR:-/var/run/ebpf_net}
dump_dir="${data_dir}/dump"
mkdir -p "${data_dir}" "${dump_dir}"

if [ -n "$HEADLOG" ]; then
  "${reducer}" "$@" 2>&1 | sed -n '1,1000000p' > /tmp/logtmp
elif [ -n "${EBPF_NET_RUN_UNDER_GDB}" ]; then
  if [[ "${#EBPF_NET_GDB_COMMANDS[@]}" -lt 1 ]]; then
    # default behavior is to run the pipeline server, print a stack trace after it exits
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

  (set -x; exec "${EBPF_NET_RUN_UNDER_GDB}" -q "${GDB_ARGS[@]}" --args "${reducer}" "$@")
else
  exec "${reducer}" "$@"
fi
