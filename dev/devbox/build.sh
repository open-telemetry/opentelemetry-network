#!/usr/bin/env -S bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


export EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
export DEVBOX_DIR="${EBPF_NET_SRC_ROOT}/dev/devbox"
export DEVBOX_BOXES_SOURCE="${DEVBOX_DIR}/source"
export DEVBOX_BOXES_DIR="${DEVBOX_DIR}/boxes"

packer_file="${DEVBOX_BOXES_SOURCE}/devbox.packer.json"

unset packer_vars
declare -A packer_vars
for var in $(jq -r '.variables|keys|.[]' < "${packer_file}"); do
  packer_vars["${var}"]="${var}"
done

function print_help {
  echo "usage:"
  echo "  $0 --base_box vagrant_box --box_name devbox_name [--variable_name value [...]]"
  echo
  echo "example:"
  echo "  $0 --base_box ubuntu/bionic64 --box_name my-dev-box"
  echo
  echo "variables:"
  for var in "${!packer_vars[@]}"; do
    echo "- ${var}"
  done
}

if [[ "$#" -eq 0 ]]; then
  print_help
  exit 0
fi

packer_args=(build)
box_name="devbox"

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift

  case "${arg}" in
    -h | --help)
      print_help
      exit 0
      ;;

    --)
      break
      ;;

    *)
      if [[ "${arg:0:2}" != "--" ]]; then
        echo "ERROR: unknown argument '${arg}'"
        echo
        print_help
        exit
      fi

      var_name="${arg:2}"

      if [[ -z "${packer_vars["${var_name}"]}" ]]; then
        echo "ERROR: unknown variable '${var_name}'"
        echo
        print_help
        exit 1
      fi

      if [[ "$#" -lt 1 ]]; then
        echo "ERROR: no value given for variable '${var_name}'"
        exit 1
      fi

      var_value="$1"; shift

      packer_args+=(-var "${var_name}=${var_value}")

      if [[ "${var_name}" = "box_name" ]]; then
        box_name="${var_value}"
      fi
      ;;
  esac
done

packer_args+=(-on-error=ask "$@")

build_dir="$(mktemp -d)"
export PACKER_LOG=1
export VERBOSE=1
(set -x; time packer "${packer_args[@]}" -var "output_dir=${build_dir}/out" "${packer_file}")
mv "${build_dir}/out/package.box" "${DEVBOX_BOXES_DIR}/${box_name}.box"
