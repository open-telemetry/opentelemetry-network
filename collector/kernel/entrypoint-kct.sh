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

if ! mountpoint -q /sys; then
  mount -t sysfs none /sys || echo "Warning: Could not mount sysfs"
fi

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
    --args "${install_dir}/kernel_collector_test" "$@" \
  )
elif [[ -n "${EBPF_NET_RUN_UNDER_VALGRIND}" ]]; then
  # to run the collector under valgrind, set `EBPF_NET_RUN_UNDER_VALGRIND` to the options to pass to valgrind,
  # including at minimum the tool you want, for example:
  # "--tool=memcheck", or
  # "--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes", or
  # "--tool=massif --stacks=yes"
  # note: to get full symbols from valgrind also build the kernel-collector in debug mode

  # shellcheck disable=SC2086
  (set -x; exec /usr/bin/valgrind ${EBPF_NET_RUN_UNDER_VALGRIND} "${install_dir}/kernel_collector_test" "$@")
else
  # Run the test binary and capture its exit code without inverting it.
  (set -x; "${install_dir}/kernel_collector_test" "$@")
  test_exit_code=$?
  if [[ ${test_exit_code} -ne 0 ]]; then
    echo "kernel collector test FAILED"
    cp /srv/core-* /hostfs/data || true
    if [[ -n "${DELAY_EXIT_ON_FAILURE}" ]]; then
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

# Copy converted dumps to host-mounted data directory for artifact collection
if [ -d /hostfs/data ]; then
  if [ -e /tmp/bpf-dump-file.json ]; then
    cp -f /tmp/bpf-dump-file.json /hostfs/data/ || true
  fi
  if [ -e /tmp/intake-dump-file.json ]; then
    cp -f /tmp/intake-dump-file.json /hostfs/data/ || true
    # Optional: emit a filtered file with only bpf_log messages from intake JSON
    jq '.[] | select(.name=="bpf_log")' /tmp/intake-dump-file.json > /hostfs/data/bpf-logs.json 2>/dev/null || true
  fi
fi

if [[ -n "${DELAY_EXIT}" ]]
then
  echo "DELAY_EXIT is set, doing 'sleep inf'"
  sleep inf
fi

# Propagate the test's exit code so the container exit reflects pass/fail.
if [[ -n "${test_exit_code:-}" ]]; then
  exit "${test_exit_code}"
fi
