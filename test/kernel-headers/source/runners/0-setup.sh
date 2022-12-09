#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

source "${EBPF_NET_SRC}/dev/script/benv_error_lib.sh"
set -x

vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
vagrant box update
vagrant up --provision

EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"

# Check if local docker registry is running
result=$(docker ps | grep registry:latest | grep local-docker-registry) || true
if [[ "${result}" == "" ]]
then
  # local registry isn't running, so start it
  "${EBPF_NET_SRC}/dev/docker-registry.sh"
fi

# Check that required docker images exist
result1=$(docker images | grep ^kernel-collector) || true
result2=$(docker images | grep ^reducer) || true
if [[ "${result1}" == "" || "${result2}" == "" ]]
then
  echo "ERROR: required docker image(s) do not exist!"
  exit 1
fi

# Push docker images to local docker registry
docker tag kernel-collector localhost:5000/kernel-collector:latest
docker push localhost:5000/kernel-collector
docker tag reducer localhost:5000/reducer:latest
docker push localhost:5000/reducer

vagrant up --no-provision

# Confirm tha the vagrant VM is running
result=$(vagrant status | grep ^default | grep running) || true
if [[ "${result}" == "" ]]
then
  echo "ERROR: vagrant VM is not running!"
  exit 1
else
  echo "vagrant VM is running!"
fi
