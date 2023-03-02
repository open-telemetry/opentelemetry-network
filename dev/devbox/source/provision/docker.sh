#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

##############
# docker setup

case "${ID}" in
  debian | ubuntu)
    export DEBIAN_FRONTEND="noninteractive"

    curl -sfL 'https://download.docker.com/linux/debian/gpg' | sudo -E apt-key add -

    if [[ -n "$(apt-cache search '^docker\.io$')" ]]; then
      sudo apt-get install -y --no-install-recommends --allow-change-held-packages docker.io
    elif [[ -n "$(apt-cache search '^docker-ce$')" ]]; then
      sudo apt-get install -y --no-install-recommends --allow-change-held-packages docker-ce
    else
      curl -fsSL https://get.docker.com/ | sudo sh
    fi
    sudo usermod -aG docker "${USER}"
    sudo systemctl enable docker
    ;;

  centos)
    sudo yum-config-manager --add-repo "https://download.docker.com/linux/centos/docker-ce.repo"
    pkg_list=(docker-ce docker-ce-cli containerd.io)
    case "${VERSION_ID}" in
      7)
        sudo yum install -y "${pkg_list}"
        ;;

      *)
        sudo yum install -y --nobest "${pkg_list}"
        ;;
    esac
    sudo usermod -aG docker "${USER}"
    sudo systemctl enable docker
    ;;
esac
