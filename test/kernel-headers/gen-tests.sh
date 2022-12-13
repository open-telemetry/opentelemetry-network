#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
source "${OTEL_EBPF_SRC}/dev/script/bash-error-lib.sh"
set -x

source ${OTEL_EBPF_SRC}/test/kernel-headers/distros-and-kernels.sh

for ((i = 0; i < ${#distros_and_kernels[@]}; i++))
do
  distro_and_kernel="${distros_and_kernels[$i]}"
  ${OTEL_EBPF_SRC}/test/kernel-headers/bootstrap.sh ${distro_and_kernel}
done
