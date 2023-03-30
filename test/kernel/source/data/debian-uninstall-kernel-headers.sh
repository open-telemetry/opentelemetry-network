#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export RUNNING_KERNEL_VERSION="`uname -r`"
export RUNNING_KERNEL_ARCH="${RUNNING_KERNEL_VERSION##*-}"

sudo apt-get purge --auto-remove --purge -y --no-install-recommends \
  --allow-unauthenticated \
  --allow-downgrades \
  --allow-remove-essential \
  --allow-change-held-packages \
  \
  "linux-headers-${RUNNING_KERNEL_VERSION}" \
  "linux-headers-${RUNNING_KERNEL_ARCH}" \
  || true
