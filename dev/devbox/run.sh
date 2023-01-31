#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
# shellcheck source=/dev/null
source "${OTEL_EBPF_SRC}/dev/script/bash-error-lib.sh"
# shellcheck source=/dev/null
source "${OTEL_EBPF_SRC}/dev/script/benv-lib.sh"

export OTEL_EBPF_BENV_OUT="$(get_benv_build_dir)"

vagrant up
vagrant ssh -- -R 5000:localhost:5000 -L 59090:localhost:9090
