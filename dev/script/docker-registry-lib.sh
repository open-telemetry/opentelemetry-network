# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

function ecr_login {
  login_command=(aws ecr get-login --no-include-email)

  if [[ "$1" == false ]]; then
    eval "$("${login_command[@]}")"
  else
    # aws-vault v6 writes the version to stderr now,
    # so redirect stderr to stdout
    aws_vault_version=$(aws-vault --version 2>&1)
    aws_vault_args=()
    aws_vault_major_version=$(echo ${aws_vault_version} | cut -d. -f1 | cut -c2-)
    # aws-vault v5 removed the --assume-role-ttl flag and replaced it with --duration
    if [[ "${aws_vault_major_version}" -ge 5 ]]; then
      aws_vault_args+=("--duration=1h")
    else
      aws_vault_args+=("--assume-role-ttl=1h")
    fi

    echo "logging in to ecr..."
    eval "$(aws-vault exec "${aws_vault_args[@]}" flowmill-prod -- "${login_command[@]}")"
    echo -e "done\\n"
  fi
}

function gcr_login {

  # Make sure docker is set up to use the credentials helpers
  gcloud auth configure-docker

  echo "logging in to gcr ..."
  gcloud auth login
  echo -e "done\\n"
}

function okta_login {
  echo "logging in to okta ..."
  okta-docker-login
  echo -e "done\\n"
}

function flowmill_detect_docker_registry {
  docker_registry="$1"; shift
  if [ -z "${docker_registry}" ]; then
    echo 'none'
  elif echo "${docker_registry}" | grep 'localhost:' > /dev/null; then
    echo 'local'
  elif echo "${docker_registry}" | grep '\.dkr\.ecr\.' > /dev/null; then
    echo 'ecr'
  elif echo "${docker_registry}" | grep 'gcr\.io' > /dev/null; then
    echo 'gcr'
  else
    echo 'unknown'
  fi
}
