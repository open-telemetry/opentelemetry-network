#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe
vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
vagrant up --provision &
EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"
"${EBPF_NET_SRC}/dev/docker-registry.sh" || true
"${EBPF_NET_SRC}/dev/push_to_local_registry.sh" agent server
wait
vagrant up --no-provision
