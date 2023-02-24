#!/bin/bash

# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

echo $DOCKER_HUB_PATH

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
  tag=$4
fi    
  
name=${distro}-${version}
[ "$kernel_version" != "" ] && name=${name}-${kernel_version}
print "Testing ${name}"
${EBPF_NET_SRC_ROOT}/test/kernel-headers/bootstrap.sh ${distro} ${version} ${kernel_version}

cd ${name}
print "running 0-setup.sh"
./0-setup.sh ${tag}
print "running 1-start-reducer.sh"
./1-start-reducer.sh ${tag} $DOCKER_HUB_PATH

## this script should only be run for SE linux. Currently only centos-7 is an SE. 
if [ "$distro" == "centos" ] 
then
  print "running 2-apply-selinux-policy.sh"
  ./2-apply-selinux-policy.sh
fi  
# Ubuntu Jammy cannot automatically fetch headers currently because kernel-collector with bitnami/minideb:buster
# base image does not support zstd compression
is_jammy=$(grep jammy <<<$version) || true
if [[ "${is_jammy}" != "" ]]
then
  print "SKIPPING 3-fetch.sh and 4-cached.sh for ${name}"
else
  print "running 3-fetch.sh"
  ./3-fetch.sh ${tag} $DOCKER_HUB_PATH
  print "running 4-cached.sh"
  ./4-cached.sh ${tag} $DOCKER_HUB_PATH
fi
print "running 5-pre-installed.sh"
./5-pre-installed.sh ${tag} $DOCKER_HUB_PATH
print "running 6-cleanup.sh"
./6-cleanup.sh
print "Testing of ${name} succeeded"
cd ..

