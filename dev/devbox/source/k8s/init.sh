#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

source /etc/os-release
if [[ "${ID}" != "ubuntu" ]]
then
  echo "k8s is currently only supported in Ubuntu devboxes."
  exit 1
fi

echo -e "\n---------- Starting microk8s ----------"
set -x
microk8s start
microk8s status --wait-ready
microk8s enable dns
microk8s enable hostpath-storage
microk8s enable registry


set +x
echo
echo -e "\nTo change the microk8s install to a different version/snap channel, for example:"
echo "  sudo snap refresh microk8s --channel=latest/edge"
echo "  OR"
echo "  sudo snap refresh microk8s --channel=1.25"
echo "To see the currently available channels:"
echo "  snap info microk8s"
echo

echo -e "\n---------- Installing stern ----------"
set -x
microk8s kubectl krew update
microk8s kubectl krew install stern

set +x
echo -e "\n---------- Installing helm diff ----------"
set -x
microk8s helm plugin install https://github.com/databus23/helm-diff

