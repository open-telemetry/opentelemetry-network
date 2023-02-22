#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

###########################################
# set up symlinks based off of EBPF_NET_SRC

source_dir="${HOME}/src/dev/devbox/source"
files=( \
  cloud-collector.sh
  collector-entrypoint.sh
  k8s
  k8s-collector.sh
  kernel-collector.sh
  otelcol-gateway.sh
  reducer.sh
  .rgrc
)

for file in "${files[@]}"; do
  ln -s "${source_dir}/${file}" "${HOME}/${file}"
done
