#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

###########################
# upgrade existing packages

case "${ID}" in
  debian | ubuntu)
    export DEBIAN_FRONTEND="noninteractive"

    sudo -E apt-get upgrade --auto-remove --purge -y \
      --no-install-recommends \
      --allow-change-held-packages
    ;;

  centos)
    case "${VERSION_ID}" in
      7)
        sudo yum update -y
        ;;

      *)
        sudo yum update -y --nobest
        ;;
    esac
    ;;
esac
