#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
"${OTEL_EBPF_SRC}/dev/devbox/build.sh" --base_box ubuntu/jammy64 --box_name ubuntu-jammy
