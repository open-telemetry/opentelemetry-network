#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$(git rev-parse --show-toplevel)}"
source "${EBPF_NET_SRC_ROOT}/dev/script/bash-error-lib.sh"
set -x

src_path="${EBPF_NET_SRC_ROOT}/test/kernel/source"

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

if [[ "${distro_name}" == "bento" && "${distro_version}" == "amazonlinux-2" ]]
then
  script_distro_name="centos"
else
  script_distro_name="${distro_name}"
fi

sed_args=( \
  -e "s/PLACEHOLDER_BOX_DISTRO_NAME/${distro_name}/g"
  -e "s/PLACEHOLDER_DISTRO_NAME/${script_distro_name}/g"
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
