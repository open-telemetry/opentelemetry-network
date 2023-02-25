#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# on systems with SELinux enabled this script should be run before starting the
# kernel-collector with 2-fetch.sh, 3-cached.sh or 4-pre-installed.sh

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

vagrant scp ${EBPF_NET_SRC_ROOT}/dev/selinux-bpf.sh /tmp
vagrant ssh -- sudo /tmp/selinux-bpf.sh



vagrant scp ${EBPF_NET_SRC_ROOT}/dev/selinux-resolv-conf.sh /tmp
vagrant ssh -- sudo /tmp/selinux-resolv-conf.sh