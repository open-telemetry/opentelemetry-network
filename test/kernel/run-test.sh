#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"

function print {
  set +x
  echo "===================================== $1 ====================================="
  set -x
}

function print_help {
  echo "usage: $0 [--all|--kernel-collector-test|--kernel-header-test] <DISTRO> <VERSION> <optional KERNEL_VERSION>"
  echo
  echo "  --all: run all tests"
  echo "  --kernel-collector-test: run the kernel_collector_test"
  echo "  --kernel-header-test: run the kernel header tests"
}

run_kernel_collector_test="false"
run_kernel_header_test="false"

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --all)
      run_kernel_collector_test="true"
      run_kernel_header_test="true"
      ;;

    --kernel-collector-test)
      run_kernel_collector_test="true"
      ;;

    --kernel-header-test)
      run_kernel_header_test="true"
      ;;

    *)
      args+=("${arg}")
      ;;
  esac
done

if [[ "${run_kernel_collector_test}" != "true" && "${run_kernel_header_test}" != "true" ]]
then
  echo -e "\nNeed to specify test(s) to run.\n"
  print_help
  exit 1
fi

distro=${args[0]}
version=${args[1]}
kernel_version=${args[2]}

if [[ "${distro}" == "" || "${version}" == "" ]]
then
  echo -e "\nNeed to specify distro and version.\n"
  print_help
  exit 1
fi

set -x

name=${distro}-${version}
[ "$kernel_version" != "" ] && name=${name}-${kernel_version}
print "Testing ${name}"
${EBPF_NET_SRC_ROOT}/test/kernel/bootstrap.sh ${distro} ${version} ${kernel_version}

cd ${name}
print "running 0-setup.sh"
./0-setup.sh

print "running 1-apply-selinux-policy.sh"
./1-apply-selinux-policy.sh

if [[ "${run_kernel_collector_test}" == "true" ]]
then
  print "running run-kernel-collector-test.sh"
  if ./run-kernel-collector-test.sh
  then
    print "kernel_collector_test on ${name} succeeded"
  else
    print "kernel_collector_test on ${name} FAILED"
    test_failed="true"
  fi
fi

if [[ "${run_kernel_header_test}" == "true" ]]
then
  print "running 2-start-reducer.sh"
  ./2-start-reducer.sh

  # Ubuntu Jammy cannot automatically fetch headers currently because kernel-collector with bitnami/minideb:buster
  # base image does not support zstd compression
  if [[ "${version}" == *"jammy"* ]]
  then
    print "SKIPPING 3-fetch.sh and 4-cached.sh for ${name}"
  else
    print "running 3-fetch.sh"
    ./3-fetch.sh
    print "running 4-cached.sh"
    ./4-cached.sh
  fi
  print "running 5-pre-installed.sh"
  ./5-pre-installed.sh

  print "Kernel header tests on ${name} succeeded"
fi

print "running 6-cleanup.sh"
./6-cleanup.sh

if [[ "${test_failed}" == "true" ]]
then
  print "Testing on ${name} FAILED"
  exit 1
else
  print "Testing on ${name} succeeded"
fi


