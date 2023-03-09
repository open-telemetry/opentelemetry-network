#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
"${EBPF_NET_SRC_ROOT}/dev/devbox/build.sh" --base_box centos/7 --box_name centos-7
