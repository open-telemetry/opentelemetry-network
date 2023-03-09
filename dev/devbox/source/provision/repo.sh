#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

#############################
# set up package repositories

case "${ID}" in
  debian)
    export DEBIAN_FRONTEND="noninteractive"

    if [[ -z "${VERSION_CODENAME}" ]]; then
      VERSION_CODENAME="unstable"
    fi

    case "${VERSION_CODENAME}" in
      unstable | *sid*)
        sudo tee /etc/apt/sources.list <<EOF
deb http://deb.debian.org/debian/ unstable main non-free contrib
EOF
        ;;

      *)
        ;;
    esac

    sudo -E apt-get update -y
    ;;

  ubuntu)
    export DEBIAN_FRONTEND="noninteractive"

    sudo -E apt-get update -y
    ;;

  centos)
    sudo yum install -y yum-utils

    case "${VERSION_ID}" in
      7)
        sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
        ;;

      8)
        sudo yum config-manager --set-enabled PowerTools
        ;;
    esac

    sudo yum list
    ;;

  *)
    echo "unknown distro"
    exit 1
    ;;
esac
