#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
set -o pipefail
set -x

echo $DOCKER_HUB_PATH

source ${EBPF_NET_SRC_ROOT}/test/kernel-headers/distros-and-kernels.sh

test_dir="$(mktemp -d /tmp/kernel-header-tests-$(date +%Y-%m-%d-%H-%M)-XXX)"
cd ${test_dir}

touch ${test_dir}/summary.log
echo "Saving test results in ${test_dir}"

num_run=0
num_failed=0
empty_placeholder="github_runner"
for ((i = 0; i < ${#distros_and_kernels[@]}; i++))
do
  num_run=$((num_run+1))
  distro_and_kernel="${distros_and_kernels[$i]}"
  log_file=${test_dir}/"$(sed 's/ /-/g' <<<$distro_and_kernel)".log
  echo "Running kernel header tests for \"${distro_and_kernel}\"." | tee -a ${test_dir}/summary.log
  if [[ $# == 1  ]]
  then
    ${EBPF_NET_SRC_ROOT}/test/kernel-headers/run-test.sh ${distro_and_kernel} $1 $empty_placeholder 2>&1 | tee ${log_file}
  else
    ${EBPF_NET_SRC_ROOT}/test/kernel-headers/run-test.sh ${distro_and_kernel} 2>&1 | tee ${log_file}
  fi

  if [[ $? == 0 ]]
  then
    echo -e "Tests succeeded for \"${distro_and_kernel}\".\n" | tee -a ${test_dir}/summary.log
  else
    num_failed=$((num_failed+1))
    echo -e "Tests FAILED for \"${distro_and_kernel}\".\n" | tee -a ${test_dir}/summary.log
  fi
done

echo -e "Ran ${num_run} tests, with ${num_failed} failure(s).\n" | tee -a ${test_dir}/summary.log

echo "Test results are in ${test_dir}.

if [[ ${num_failed} -gt 0 ]]
then
    echo "One or more test(s) failed."
    exit 1
fi

