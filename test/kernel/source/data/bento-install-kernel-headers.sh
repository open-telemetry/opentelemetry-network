#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export RUNNING_KERNEL_VERSION="`uname -r`"


if ! grep 'ID="amzn"' /etc/os-release
then
  # rocky linux
  sudo yum install -y dnf-utils
  sudo yum config-manager --set-enabled devel
  sudo yum install -y kernel-devel-"${RUNNING_KERNEL_VERSION}"
else
  sudo yum install -y kernel-devel "${RUNNING_KERNEL_VERSION}"
fi

