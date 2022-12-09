#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

uname -a

export kernel_version="4.15.0-1037-aws"

apt-get update -y
apt-get install -y --no-install-recommends \
  docker.io

apt-get install -y --no-install-recommends \
  --allow-unauthenticated \
  --allow-downgrades \
  --allow-remove-essential \
  --allow-change-held-packages \
  \
  "linux-image-${kernel_version}"

if [ "`uname -r`" != "${kernel_version}" ]; then
  apt-get purge --auto-remove --purge -y --no-install-recommends \
    --allow-unauthenticated \
    --allow-downgrades \
    --allow-remove-essential \
    --allow-change-held-packages \
    \
    linux-headers-`uname -r` \
    linux-image-`uname -r` \
    linux-headers-generic \
    linux-headers-virtual \
    linux-image-virtual \
    linux-virtual \
    || true
fi

apt-get purge --auto-remove --purge -y --no-install-recommends \
  --allow-unauthenticated \
  --allow-downgrades \
  --allow-remove-essential \
  --allow-change-held-packages \
  \
  "linux-headers-${kernel_version}" \
  || true

usermod -aG docker vagrant
systemctl enable docker

mkdir -p /var/cache/ebpf_net

chmod +x server.sh
chmod +x agent.sh

cat >> .env <<EOF
export EBPF_NET_INTAKE_PORT="${EBPF_NET_INTAKE_PORT}"
export EBPF_NET_INTAKE_HOST="${EBPF_NET_INTAKE_HOST}"
export EBPF_NET_AGENT_NAMESPACE="${EBPF_NET_AGENT_NAMESPACE}"
export EBPF_NET_AGENT_CLUSTER="${EBPF_NET_AGENT_CLUSTER}"
export EBPF_NET_AGENT_SERVICE="${EBPF_NET_AGENT_SERVICE}"
export EBPF_NET_AGENT_HOST="${EBPF_NET_AGENT_HOST}"
export EBPF_NET_AGENT_ZONE="${EBPF_NET_AGENT_ZONE}"
export EBPF_NET_KERNEL_HEADERS_AUTO_FETCH="${EBPF_NET_KERNEL_HEADERS_AUTO_FETCH}"
EOF

shutdown -r now
