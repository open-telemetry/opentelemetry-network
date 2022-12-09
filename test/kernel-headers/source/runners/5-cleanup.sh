#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

source "${EBPF_NET_SRC}/dev/script/benv_error_lib.sh"
set -x

vagrant destroy -f || true
[ -e .vagrant ] && rm -rf .vagrant
