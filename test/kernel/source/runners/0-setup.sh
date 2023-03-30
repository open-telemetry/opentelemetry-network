#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

echo $1

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
set -x


vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
vagrant box update
vagrant up --provision
if [[ $? != 0 ]]
then
    echo "vagrant up failed"
    vagrant destroy -f
    exit 1
fi


# don't do this until after vagrant up so the script can check the result and cleanup and exit if it fails
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"

vagrant up --no-provision

# Confirm that the vagrant VM is running
result=$(vagrant status | grep ^default | grep running) || true
if [[ "${result}" == "" ]]
then
  echo "ERROR: vagrant VM is not running!"
  vagrant status
  exit 1
else
  echo "vagrant VM is running!"
fi

# Sometimes vagrant ssh, as run by subsequent test runners, fails immediately after vagrant up, so wait until it works successfully.
ready_string="ready to ssh"
remaining_attempts=10
while true
do
  out=$(vagrant ssh -- -R "5000:localhost:5000" -- echo "${ready_string}") || true
  if [[ "${out}" == "${ready_string}" ]]
  then
    break
  fi

  remaining_attempts=$(($remaining_attempts-1))
  if [[ $remaining_attempts == 0 ]]
  then
    vagrant status
    vagrant ssh-config
    echo "ERROR: vagrant VM is not accepting ssh requests!"
    exit 1
  fi

  sleep 1
done
