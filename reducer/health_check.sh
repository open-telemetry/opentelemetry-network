#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set -e

function print_little_endian_bytes {
  if [[ "$#" -ne 2 ]]; then exit 1; fi
  number="$1"; shift
  byte_pad="$1"; shift
  padding_size="$((byte_pad * 2))"
  hex_number="$(echo "obase=16;ibase=10;${number}"|bc)"
  padded_hex_number="$(printf "%${padding_size}s" "${hex_number}" | tr ' ' 0)"
  little_endian_hex_number="$(echo "${padded_hex_number}" | rev | sed -e 's/\(.\)\(.\)/\2\1/g')"
  # bash regexes aren't cutting it so we go with sed
  # shellcheck disable=SC2001
  formatted_hex_number="$(echo "${little_endian_hex_number}" | sed -e 's/../\\x&/g')"
  # string contains format directives for printf so don't use "%s"
  # shellcheck disable=SC2059
  printf "${formatted_hex_number}"
}

function build_health_check_packet {
  fixed_message_size=5

  if [[ "$#" -ne 2 ]]; then exit 1; fi
  client_type="$1"; shift
  location="$1"; shift
  location_size="${#location}"

  print_little_endian_bytes "$(date +%s%N)" 8
  print_little_endian_bytes 409 2 # health check rpc id
  print_little_endian_bytes "$((fixed_message_size + location_size))" 2
  print_little_endian_bytes "${client_type}" 1
  echo -n "${location}"
}

if [[ "$#" -ne 3 ]]; then exit 1; fi

case "$1" in
  liveness_probe)
    client_type=7
    ;;
  readiness_probe)
    client_type=8
    ;;
  *)
    echo "unsupported client type: '$1'"
    exit 1
    ;;
esac

location="$2"
server_port="$3"

build_health_check_packet "${client_type}" "${location}" \
  | nc -q 1 localhost "${server_port}"
