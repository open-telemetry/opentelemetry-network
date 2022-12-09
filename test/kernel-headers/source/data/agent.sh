#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

container_id_file="/tmp/container.id"
if [[ -e "${container_id_file}" ]]; then
  docker stop "$(cat "${container_id_file}")"
  docker rm "$(cat "${container_id_file}")"
  rm "${container_id_file}"
fi

docker pull localhost:5000/kernel-collector

docker create \
  --env EBPF_NET_INTAKE_PORT="8000" \
  --env EBPF_NET_INTAKE_HOST="127.0.0.1" \
  --env EBPF_NET_AGENT_NAMESPACE="${EBPF_NET_AGENT_NAMESPACE}" \
  --env EBPF_NET_AGENT_CLUSTER="${EBPF_NET_AGENT_CLUSTER}" \
  --env EBPF_NET_AGENT_SERVICE="${EBPF_NET_AGENT_SERVICE}" \
  --env EBPF_NET_AGENT_HOST="${EBPF_NET_AGENT_HOST}" \
  --env EBPF_NET_AGENT_ZONE="${EBPF_NET_AGENT_ZONE}" \
  --env EBPF_NET_KERNEL_HEADERS_AUTO_FETCH="true" \
  --env EBPF_NET_HOST_DIR="/hostfs" \
  --privileged \
  --pid host \
  --network host \
  --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup \
  --volume /usr/src:/hostfs/usr/src \
  --volume /lib/modules:/hostfs/lib/modules \
  --volume /etc:/hostfs/etc \
  --volume /var/cache:/hostfs/cache \
  --volume /var/run/docker.sock:/var/run/docker.sock \
  --entrypoint "/srv/test-entrypoint.sh" \
  localhost:5000/kernel-collector \
    --log-console \
    --debug \
  > "${container_id_file}"

export container_id="$(cat "${container_id_file}")"

docker cp ".env" "${container_id}:/srv/.env"
docker cp "test-entrypoint.sh" "${container_id}:/srv/test-entrypoint.sh"

docker start -i "${container_id}"

rm "${container_id_file}"