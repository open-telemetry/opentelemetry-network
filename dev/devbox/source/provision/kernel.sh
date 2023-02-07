#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

################################
# install kernel and its headers

case "${ID}" in
  debian)
    export DEBIAN_FRONTEND="noninteractive"

    if [[ -z "${VERSION_CODENAME}" ]]; then
      VERSION_CODENAME="unstable"
    fi

    case "${VERSION_CODENAME}" in
      unstable | *sid*)
        # workardound for the current situation of debian-sid
        sudo -E apt-get purge -y libgcc1
        ;;
    esac

    sudo -E apt-cache search linux-headers
    sudo -E apt-get install -y --allow-change-held-packages \
      linux-image-amd64 linux-headers-amd64 \
      dkms kmod build-essential
    ;;

  ubuntu)
    export DEBIAN_FRONTEND="noninteractive"

    pkg_list=( \
      linux-virtual linux-image-virtual linux-headers-virtual
      dkms kmod build-essential
    )

    sudo -E apt-get install -y --allow-change-held-packages "${pkg_list[@]}"
    ;;

  centos)
    sudo yum install -y kernel kernel-devel kernel-headers
    ;;
esac
