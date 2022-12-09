#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

apt-get update -y

apt-get install -y --no-install-recommends \
  docker.io

usermod -aG docker vagrant
systemctl enable docker

export RUNNING_KERNEL_VERSION="`uname -r`"
export RUNNING_KERNEL_ARCH="${RUNNING_KERNEL_VERSION##*-}"

if [[ -n "${KERNEL_VERSION}" ]]; then
  apt-get install -y --no-install-recommends \
    --allow-unauthenticated \
    --allow-downgrades \
    --allow-remove-essential \
    --allow-change-held-packages \
    \
    "linux-image-${KERNEL_VERSION}"

  if [ "${RUNNING_KERNEL_VERSION}" != "${KERNEL_VERSION}" ]; then
    apt-get purge --auto-remove --purge -y --no-install-recommends \
      --allow-unauthenticated \
      --allow-downgrades \
      --allow-remove-essential \
      --allow-change-held-packages \
      \
      "linux-image-${RUNNING_KERNEL_VERSION}" \
      "linux-image-virtual" \
      "linux-virtual" \
      || true
  fi
else
  apt-get upgrade --auto-remove --purge -y --no-install-recommends \
    --allow-unauthenticated \
    --allow-downgrades \
    --allow-remove-essential \
    --allow-change-held-packages \
    \
    "linux-virtual"
fi
