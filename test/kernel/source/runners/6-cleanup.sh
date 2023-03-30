#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
