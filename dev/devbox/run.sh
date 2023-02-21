#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
# shellcheck source=/dev/null
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
# shellcheck source=/dev/null
source "${EBPF_NET_SRC_ROOT}/dev/script/benv-lib.sh"

export EBPF_NET_OUT_DIR="$(get_benv_build_dir)"

vagrant up
vagrant ssh -- -R 5000:localhost:5000 -L 58080:localhost:8080 -L 59090:localhost:9090
