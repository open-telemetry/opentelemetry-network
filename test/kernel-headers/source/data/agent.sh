#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe
echo $DOCKER_HUB_PATH

image_loc="localhost:5000/kernel-collector"
container_name="test-kernel-collector"

if [ "$(docker ps -a -q -f name="${container_name}")" ]
then
  docker stop "${container_name}"
  docker rename "${container_name}" "${container_name}-stopped"
  docker rm "${container_name}-stopped"
fi

if [ $# -eq 0 ]
then
  sudo docker pull ${image_loc}
elif [ $# -eq 2 ]
then
  tag=":$1"
  docker_hub_path="$2"
  image_loc="$docker_hub_path/kernel-collector${tag}"
  echo $image_loc
  sudo docker pull ${image_loc}    
fi


docker create \
  --name "${container_name}" \
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
  "${image_loc}" \
    --log-console \
    --debug

docker cp ".env" "${container_name}:/srv/.env"
docker cp "test-entrypoint.sh" "${container_name}:/srv/test-entrypoint.sh"

docker start "${container_name}"

start_string="Telemetry is flowing\!"

remaining_attempts=24
while true
do
  result=$(docker ps | grep "${container_name}") || true
  if [[ "${result}" == "" ]]
  then
    docker ps -a
    docker logs "${container_name}"
    echo "ERROR: kernel-collector container is not running!"
    exit 1
  fi

  result=$(docker logs "${container_name}" | grep "${start_string}") || true
  if [[ "${result}" != "" ]]
  then
    break
  fi

  remaining_attempts=$(($remaining_attempts-1))
  if [[ $remaining_attempts == 0 ]]
  then
    docker ps -a
    docker logs "${container_name}"
    echo "ERROR: kernel-collector did not start within the time expected!"
    exit 1
  fi

  sleep 5
done

echo "kernel-collector is running!"
