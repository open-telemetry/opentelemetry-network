#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
# shellcheck source=/dev/null
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
# shellcheck source=/dev/null
source "${EBPF_NET_SRC_ROOT}/dev/script/benv-lib.sh"

docker exec "$(get_benv_container_name)" "${@}"

