#!/bin/bash

# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

function print {
  echo "===================================== $1 ====================================="
}

# Takes 2 to 3 arguments: distro, version, and optional kernel version
distro=$1
version=$2
kernel_version=$3

name=${distro}-${version}
[ "$kernel_version" != "" ] && name=${name}-${kernel_version}
print "Running kernel-collector-test on ${name}"
${EBPF_NET_SRC_ROOT}/test/kernel-headers/bootstrap.sh ${distro} ${version} ${kernel_version}

cd ${name}
print "running 0-setup.sh"
./0-setup.sh

print "running 2-apply-selinux-policy.sh"
./2-apply-selinux-policy.sh

print "running run-kernel-collector-test.sh"
if ! ./run-kernel-collector-test.sh
then
  test_failed="true"
fi

print "running 6-cleanup.sh"
./6-cleanup.sh

if [[ "${test_failed}" == "true" ]]
then
  print "Testing of ${name} FAILED"
  exit 1
else
  print "Testing of ${name} succeeded"
fi
