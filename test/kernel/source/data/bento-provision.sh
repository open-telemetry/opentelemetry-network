#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

yum list

yum install -y \
  curl \
  openssl

if ! grep 'ID="amzn"' /etc/os-release
then
  curl -fsSL https://get.docker.com/ | sh
else
  # get.docker.com does not currently support amazon linux
  yum install -y docker
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
