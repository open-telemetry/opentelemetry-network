#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
# shellcheck source=/dev/null
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"

# determine benv container IDs
container_ids=$(docker ps | grep -E "build-env|benv-final" | awk '{print $1}') || true
if [ -z "${container_ids}" ]
then
  echo "no running benv containers found"
  exit 1
fi

echo -e "CONTAINER_NAME\\t\\tCONTAINER_ID\\tBENV_BUILD_DIR\\t\\t\\tIMAGE"
for container_id in ${container_ids}
do
  container_name=$(docker inspect "${container_id}" | jq .[].Name | sed 's/\///' | sed 's/"//g')
  benv_build_dir=$(docker inspect "${container_id}" | jq .[].Mounts | grep tmp | awk '{print $2}' | sed 's/,$//' | sed 's/"//g')
  image=$(docker inspect "${container_id}" | jq .[].Config.Image | sed 's/"//g')
  echo -e "${container_name}\\t${container_id}\\t${benv_build_dir}\\t${image}"
done


