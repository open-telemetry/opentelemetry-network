#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"

function print_help {
  echo "usage: $0 image tag docker_registry [options...]"
  echo
  echo "options:"
  echo "  --no-login: do not log in to the docker registry"
}

args=()
login_args=()
do_login=true

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift

  case "${arg}" in
    --no-login)
      do_login=false
      ;;

    --no-vault)
      login_args+=("${arg}")
      ;;

    *)
      args+=("${arg}")
      ;;
  esac
done

image_name="${args[0]}"
image_tag="${args[1]}"

# take docker registry from command line, otherwise get it from env var EBPF_NET_DOCKER_REGISTRY
[[ "${#args[@]}" -lt 3 ]] \
  && docker_registry="${EBPF_NET_DOCKER_REGISTRY}" \
  || docker_registry="${args[2]}"
# default to local docker registry if no registry given or found
[[ -n "${docker_registry}" ]] \
  || docker_registry="localhost:5000"

if [[ "${do_login}" == true ]]; then
  "${EBPF_NET_SRC_ROOT}/dev/docker-registry-login.sh" "${login_args}" "${docker_registry}"
fi

# add --tls-verify=false for default registry (localhost:5000)
if [[ "${docker_registry}" == "localhost:5000" ]]; then
  push_args="--tls-verify=false"
else
  push_args=""
fi

(set -x; \
  podman tag "${image_name}:${image_tag}" \
    "${docker_registry}/${image_name}:${image_tag}"; \
  podman push ${push_args} "${docker_registry}/${image_name}:${image_tag}"; \
)
