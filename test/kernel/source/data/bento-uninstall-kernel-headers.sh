#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export RUNNING_KERNEL_VERSION="`uname -r`"
export RUNNING_KERNEL_ARCH="${RUNNING_KERNEL_VERSION##*-}"

sudo yum autoremove -y kernel-devel "${RUNNING_KERNEL_VERSION}" || true
sudo yum autoremove -y kernel-headers "${RUNNING_KERNEL_VERSION}" || true
sudo yum autoremove -y kernel-devel "${RUNNING_KERNEL_ARCH}" || true
sudo yum autoremove -y kernel-headers "${RUNNING_KERNEL_ARCH}" || true
