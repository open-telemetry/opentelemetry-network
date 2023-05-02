#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
set -o pipefail

function print_help {
  echo "usage: $0 [--all|--kernel-collector-test|--kernel-header-test]"
  echo
  echo "  --all run all tests on all distros/kernels"
  echo "  --kernel-collector-test: run the kernel_collector_test on all distros/kernels"
  echo "  --kernel-header-test: run the kernel header tests on all distros/kernels"
}

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --all)
      test_args="--all"
      ;;

    --kernel-collector-test)
      test_args="--kernel-collector-test"
      ;;

    --kernel-header-test)
      test_args="--kernel-header-test"
      ;;

    *)
      print_help
      exit 1
      ;;
  esac
done

if [[ "${test_args}" == "" ]]
then
  echo -e "\nNeed to specify test(s) to run.\n"
  print_help
  exit 1
fi

set -x

source ${EBPF_NET_SRC_ROOT}/test/kernel/distros-and-kernels.sh

test_dir="$(mktemp -d /tmp/kernel-tests-$(date +%Y-%m-%d-%H-%M)-XXX)"
cd ${test_dir}

touch ${test_dir}/summary.log
echo "Saving test results in ${test_dir}"

num_run=0
num_failed=0

for ((i = 0; i < ${#distros_and_kernels[@]}; i++))
do
  num_run=$((num_run+1))
  distro_and_kernel="${distros_and_kernels[$i]}"
  log_file=${test_dir}/"$(sed 's/ /-/g' <<<$distro_and_kernel)".log

  echo "Running tests (${test_args}) for \"${distro_and_kernel}\"." | tee -a ${test_dir}/summary.log
  ${EBPF_NET_SRC_ROOT}/test/kernel/run-test.sh ${test_args} ${distro_and_kernel} 2>&1 | tee ${log_file}

  if [[ $? == 0 ]]
  then
    echo -e "Test succeeded for \"${distro_and_kernel}\".\n" | tee -a ${test_dir}/summary.log
  else
    num_failed=$((num_failed+1))
    echo -e "Test FAILED for \"${distro_and_kernel}\".\n" | tee -a ${test_dir}/summary.log
  fi

done

set +x

echo -e "Ran tests (${test_args}) on ${num_run} distros/kernels, with ${num_failed} failure(s).\n" | tee -a ${test_dir}/summary.log

echo "Test results are in ${test_dir}"

if [[ ${num_failed} -gt 0 ]]
then
    echo "One or more test(s) failed."
    exit 1
fi

