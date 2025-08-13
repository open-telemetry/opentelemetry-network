#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


# shellcheck disable=SC1091
[[ ! -e ./debug-info.conf ]] || source ./debug-info.conf

if [[ "${EBPF_NET_DEBUG_MODE}" == true ]]; then
  echo "===================== /etc/os-release ====================="
  [[ ! -e /etc/os-release ]] || cat /etc/os-release
  echo "========================= uname -a ========================"
  uname -a
  echo "======================= environment ======================="
  env | sort
  echo "==========================================================="
fi

install_dir=${EBPF_NET_INSTALL_DIR:-/srv}

data_dir=${EBPF_NET_DATA_DIR:-/var/run/ebpf_net}
dump_dir="${data_dir}/dump"
mkdir -p "${data_dir}" "${dump_dir}"

# With libbpf, kernel headers are no longer needed

if ! mountpoint -q /sys; then
  mount -t sysfs none /sys || echo "Warning: Could not mount sysfs"
fi

cmd_args=()

echo "launching kernel collector..."
# on Debug (non-production) images, devs can run in local mode by setting
# `EBPF_NET_RUN_LOCAL` to non-empty.
if [[ -n "${EBPF_NET_RUN_LOCAL}" ]]; then
  # shellcheck disable=SC1091
  source /srv/local.sh
  cmd_args+=("${local_cmd_args[@]}")
fi

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
    --args "${install_dir}/kernel-collector" "${cmd_args[@]}" "$@" \
  )
elif [[ -n "${EBPF_NET_RUN_UNDER_VALGRIND}" ]]; then
  # to run the collector under valgrind, set `EBPF_NET_RUN_UNDER_VALGRIND` to the options to pass to valgrind,
  # including at minimum the tool you want, for example:
  # "--tool=memcheck", or
  # "--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes", or
  # "--tool=massif --stacks=yes"
  # note: to get full symbols from valgrind also build the kernel-collector in debug mode
  apt update -y
  apt install -y valgrind

  # shellcheck disable=SC2086
  (set -x; exec /usr/bin/valgrind ${EBPF_NET_RUN_UNDER_VALGRIND} "${install_dir}/kernel-collector" "${cmd_args[@]}" "$@")
else
  (set -x; exec "${install_dir}/kernel-collector" "${cmd_args[@]}" "$@")
fi
