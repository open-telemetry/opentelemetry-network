#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

if [[ -e "container.id" ]]; then
  docker kill "$(cat container.id)"
  rm container.id
fi

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
  --volume /usr/src:/hostfs/usr/src \
  --volume /lib/modules:/hostfs/lib/modules \
  --volume /etc:/hostfs/etc \
  --volume /var/cache:/hostfs/cache \
  --volume /var/run/docker.sock:/var/run/docker.sock \
  --entrypoint "/srv/test-entrypoint.sh" \
  localhost:5000/flowmill-agent \
    --log-console \
    --debug \
  > container.id

export container_id="$(cat container.id)"

docker cp ".env" "${container_id}:/srv/.env"
docker cp "test-entrypoint.sh" "${container_id}:/srv/test-entrypoint.sh"

docker start -i "${container_id}"
