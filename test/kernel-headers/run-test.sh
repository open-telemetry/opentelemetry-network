#!/bin/bash

# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

function print {
  echo "===================================== $1 ====================================="
}

# Takes 2 to 4 arguments: distro, version, and optional kernel version, tag
distro=$1
version=$2
if [ $# -eq 3 ];
then
  kernel_version=$3
elif [ $# -eq 4 ] || [ $# -eq 5 ];
then
  arg=$4
fi

name=${distro}-${version}
[ "$kernel_version" != "" ] && name=${name}-${kernel_version}
print "Testing ${name}"
${EBPF_NET_SRC_ROOT}/test/kernel-headers/bootstrap.sh ${distro} ${version} ${kernel_version}

cd ${name}
print "running 0-setup.sh"
./0-setup.sh ${arg}

## This should only be run for CentOS and other distributions using NetworkManager instead of static network configuration.

if [ "$distro" == "centos" ]
then
  vagrant scp ${EBPF_NET_SRC_ROOT}/dev/networkmanager-resolv-conf.sh /tmp
  vagrant ssh -- sudo /tmp/networkmanager-resolv-conf.sh
fi

print "running 1-start-reducer.sh"
./1-start-reducer.sh

print "running 2-apply-selinux-policy.sh"
  ./2-apply-selinux-policy.sh

# Ubuntu Jammy cannot automatically fetch headers currently because kernel-collector with bitnami/minideb:buster
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

