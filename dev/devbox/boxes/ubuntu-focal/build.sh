#!/bin/sh
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"
"${EBPF_NET_SRC}/dev/devbox/build.sh" --base_box ubuntu/focal64 --box_name ubuntu-focal
