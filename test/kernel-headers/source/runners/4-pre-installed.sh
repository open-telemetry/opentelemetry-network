#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

source "${EBPF_NET_SRC}/dev/script/benv_error_lib.sh"
set -x

vagrant ssh -- -- sudo rm -rf /var/cache/ebpf_net/kernel-headers || true
vagrant ssh -- -- ./install-kernel-headers.sh
vagrant ssh -- -R "5000:localhost:5000" -- ./agent.sh
