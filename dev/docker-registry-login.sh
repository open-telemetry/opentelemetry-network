#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"

source "${EBPF_NET_SRC}/dev/script/docker-registry-lib.sh"

if [ "$#" -lt 1 ]; then
  echo "usage: $0 [options...] registry..."
  echo
  echo "registry:"
  echo "  ecr: log in to ECR"
  echo "  gcr: log in to GCR"
  echo "  okta: login through the okta plugin"
  echo "  env: log in to the docker registry auto-detected off the env var EBPF_NET_DOCKER_REGISTRY"
  echo "       if the variable is unset, no login attempts are made"
  echo "  the docker registry URL can also be given, in which case its type will be auto-detected"
  echo
  echo "options:"
  echo "  --no-vault: do not use a vault app to fetch secrets from"
  echo 
  echo "ERROR: docker regsitry not specified"
fi

use_vault=true

function auto_detected_login {
  case "$(flowmill_detect_docker_registry "$1")" in
    none)
      echo "no docker registry configured, skipping login"
      ;;

    local)
      echo "no login needed for local docker registry '${EBPF_NET_DOCKER_REGISTRY}'"
      ;;

    ecr)
      echo "ECR detected at '${EBPF_NET_DOCKER_REGISTRY}'"
      ecr_login "${use_vault}"
      ;;

    gcr)
      echo "GCR detected at '${EBPF_NET_DOCKER_REGISTRY}'"
      gcr_login
      ;;

    okta)
      echo "okta detected at '${EBPF_NET_DOCKER_REGISTRY}'"
      okta_login
      ;;


    *)
      echo "ERROR: unrecognized docker registry '${EBPF_NET_DOCKER_REGISTRY}'"
      return 1
      ;;
  esac
}

while [ "$#" -gt 0 ]; do
  arg="$1"; shift

  case "${arg}" in
    --no-vault)
      use_vault=false
      ;;

    ecr)
      ecr_login "${use_vault}"
      ;;

    gcr)
      gcr_login
      ;;

    okta)
      okta_login
      ;;

    env)
      auto_detected_login "${EBPF_NET_DOCKER_REGISTRY}"
      ;;

    *)
      auto_detected_login "${arg}"
      ;;
      
  esac
done
