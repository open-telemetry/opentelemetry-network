#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export RUNNING_KERNEL_VERSION="`uname -r`"
export DEBIAN_FRONTEND="noninteractive"

sudo apt-get install -y --no-install-recommends \
  --allow-unauthenticated \
  --allow-downgrades \
  --allow-remove-essential \
  --allow-change-held-packages \
  \
  "linux-headers-${RUNNING_KERNEL_VERSION}"
