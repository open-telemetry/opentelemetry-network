#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

vagrant ssh -- -- sudo rm -rf /var/cache/ebpf_net/kernel-headers || true
vagrant ssh -- -- ./install-kernel-headers.sh

if [ $# -eq 2 ]
then
    vagrant ssh -- -R "5000:localhost:5000" -- ./agent.sh $1 $2
elif [ $# -eq 0 ]    
then
    vagrant ssh -- -R "5000:localhost:5000" -- ./agent.sh      
fi

