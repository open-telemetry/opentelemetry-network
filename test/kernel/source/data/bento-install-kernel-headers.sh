#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export RUNNING_KERNEL_VERSION="`uname -r`"

dnf config-manager --set-enabled devel

sudo yum install -y kernel-devel "${RUNNING_KERNEL_VERSION}"
