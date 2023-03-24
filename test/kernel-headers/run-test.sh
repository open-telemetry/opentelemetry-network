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
print "Testing ${name}"
${EBPF_NET_SRC_ROOT}/test/kernel-headers/bootstrap.sh ${distro} ${version} ${kernel_version}

cd ${name}
print "running 0-setup.sh"
./0-setup.sh
print "running 1-start-reducer.sh"
./1-start-reducer.sh
print "running 2-apply-selinux-policy.sh"
./2-apply-selinux-policy.sh
# Ubuntu Jammy cannot automatically fetch headers currently because kernel-collector with bitnami/minideb:bullseye
# base image does not support zstd compression
is_jammy=$(grep jammy <<<$version) || true
if [[ "${is_jammy}" != "" ]]
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
print "running 6-cleanup.sh"
./6-cleanup.sh
print "Testing of ${name} succeeded"
cd ..

