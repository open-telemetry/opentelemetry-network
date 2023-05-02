#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

source ${EBPF_NET_SRC_ROOT}/test/kernel/distros-and-kernels.sh

for ((i = 0; i < ${#distros_and_kernels[@]}; i++))
do
  distro_and_kernel="${distros_and_kernels[$i]}"
  ${EBPF_NET_SRC_ROOT}/test/kernel/bootstrap.sh ${distro_and_kernel}
done
