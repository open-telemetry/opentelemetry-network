#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

OTEL_EBPF_SRC="${OTEL_EBPF_SRC:-$(git rev-parse --show-toplevel)}"
source "${OTEL_EBPF_SRC}/dev/script/bash-error-lib.sh"
set -x

src_path="${OTEL_EBPF_SRC}/test/kernel-headers/source"

distro_name="$1"; shift
distro_version="$1"; shift
kernel_version=""
if [[ "$#" -gt 0 ]]; then
  kernel_version="$1"; shift
fi

if [[ -z "${distro_name}" ]] || [[ -z "${distro_version}" ]]; then
  echo "usage: $0 distro_name distro_version [kernel_version]"
fi

distro_path="${distro_name}-${distro_version}"
if [[ -n "${kernel_version}" ]]; then
  distro_path="${distro_path}-${kernel_version}"
fi

mkdir -p "${distro_path}"
pushd "${distro_path}"

sed_args=( \
  -e "s/PLACEHOLDER_DISTRO_NAME/${distro_name}/g"
  -e "s/PLACEHOLDER_DISTRO_VERSION/${distro_version}/g"
  -e "s/PLACEHOLDER_KERNEL_VERSION/${kernel_version}/g"
)

sed "${sed_args[@]}" "${src_path}/Vagrantfile" > "Vagrantfile"

if [ ! -L ./data ]
then
  ln -s "${src_path}/data" "data"
fi

for runner in ${src_path}/runners/*.sh; do
  ln -sf "${runner}"
done

popd
