#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


echo "===================== /etc/os-release ====================="
[[ ! -e /etc/os-release ]] || cat /etc/os-release
echo "========================= uname -a ========================"
uname -a
echo "======================= environment ======================="
env | sort
echo "==========================================================="

install_dir=${EBPF_NET_INSTALL_DIR:-/srv}

data_dir=${EBPF_NET_DATA_DIR:-/var/run/ebpf_net}
dump_dir="${data_dir}/dump"
mkdir -p "${data_dir}" "${dump_dir}"

mount -t debugfs none /sys/kernel/debug
mount -t sysfs none /sys

cmd_args=( \
  --host-distro "${host_distro:-unknown}"
  --kernel-headers-source "${kernel_headers_source:-unknown}"
)

echo "launching kernel collector test ..."

# to run the collector under gdb, set `EBPF_NET_RUN_UNDER_GDB` to the flavor of gdb
# you want (e.g.: `cgdb` or `gdb`) - this is intended for development purposes
if [[ -n "${EBPF_NET_RUN_UNDER_GDB}" ]]; then
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
    --args "${install_dir}/kernel_collector_test" "${cmd_args[@]}" "$@" \
  )
elif [[ -n "${EBPF_NET_RUN_UNDER_VALGRIND}" ]]; then
  # to run the collector under valgrind, set `EBPF_NET_RUN_UNDER_VALGRIND` to the options to pass to valgrind,
  # including at minimum the tool you want, for example:
  # "--tool=memcheck", or
  # "--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes", or
  # "--tool=massif --stacks=yes"
  # note: to get full symbols from valgrind also build the kernel-collector in debug mode

  # shellcheck disable=SC2086
  (set -x; exec /usr/bin/valgrind ${EBPF_NET_RUN_UNDER_VALGRIND} "${install_dir}/kernel_collector_test" "${cmd_args[@]}" "$@")
else
  if ! (set -x; exec "${install_dir}/kernel_collector_test" "${cmd_args[@]}" "$@")
  then
    echo "kernel collector test FAILED"
    cp /srv/core-* /hostfs/data || true
    if [[ -n "${DELAY_EXIT_ON_FAILURE}" ]]
    then
      echo "DELAY_EXIT_ON_FAILURE is set, doing 'sleep inf'"
      sleep inf
    fi
  fi
fi

if [ -e /tmp/bpf-dump-file ]
then
  "${install_dir}/bpf_wire_to_json" < /tmp/bpf-dump-file | jq . > /tmp/bpf-dump-file.json
fi

if [ -e /tmp/intake-dump-file ]
then
  "${install_dir}/intake_wire_to_json" < /tmp/intake-dump-file | jq . > /tmp/intake-dump-file.json
fi

if [[ -n "${DELAY_EXIT}" ]]
then
  echo "DELAY_EXIT is set, doing 'sleep inf'"
  sleep inf
fi
