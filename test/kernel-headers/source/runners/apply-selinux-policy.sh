#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# on systems with SELinux enabled this script should be run before starting the
# kernel-collector with 2-fetch.sh, 3-cached.sh or 4-pre-installed.sh

source "${EBPF_NET_SRC}/dev/script/benv_error_lib.sh"
set -x

vagrant scp ${EBPF_NET_SRC}/tools/selinux_bpf.sh /tmp
vagrant ssh -- sudo /tmp/selinux_bpf.sh
