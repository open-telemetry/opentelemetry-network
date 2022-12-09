#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# on systems with SELinux enabled this script should be run before starting the
# kernel-collector with 2-fetch.sh, 3-cached.sh or 4-pre-installed.sh

OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
source "${OTEL_EBPF_SRC}/dev/script/bash-error-lib.sh"
set -x

vagrant scp ${OTEL_EBPF_SRC}/dev/selinux-bpf.sh /tmp
vagrant ssh -- sudo /tmp/selinux-bpf.sh
