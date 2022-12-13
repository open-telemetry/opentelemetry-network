#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
source "${OTEL_EBPF_SRC}/dev/script/bash-error-lib.sh"
set -x

vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
