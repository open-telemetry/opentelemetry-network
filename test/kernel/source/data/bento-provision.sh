#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

yum list

yum install -y \
  curl \
  openssl

if ! grep 'ID="amzn"' /etc/os-release && ! grep 'ID="rocky"' /etc/os-release
then
  curl -fsSL https://get.docker.com/ | sh
else
  # get.docker.com does not currently support rocky linux
  dnf check-update
  dnf config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
  dnf install docker-ce docker-ce-cli containerd.io
  systemctl start docker
  systemctl status docker
  systemctl enable docker
fi

usermod -aG docker vagrant
systemctl enable docker

export RUNNING_KERNEL_VERSION="`uname -r`"
export RUNNING_KERNEL_ARCH="${RUNNING_KERNEL_VERSION##*-}"

if [[ -n "${KERNEL_VERSION}" ]]; then
  yum install -y kernel-"${KERNEL_VERSION}"

  if [ "${RUNNING_KERNEL_VERSION}" != "${KERNEL_VERSION}" ]; then
    yum autoremove -y kernel-"${RUNNING_KERNEL_VERSION}" || true
    yum autoremove -y kernel-"${RUNNING_KERNEL_ARCH}" || true
  fi
else
  yum update -y kernel
fi
