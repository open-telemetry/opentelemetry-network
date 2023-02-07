#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

##################
# install packages

common_debian_packages=( \
  curl ca-certificates openssl net-tools iproute2 netcat-openbsd dnsutils
  policycoreutils
  openssh-server sshfs gnupg
  htop aptitude strace lsof
  tmux vim-nox
  openssh-server sshfs
  python3-bcrypt
  bc jq
  apt-transport-https
  git
  valgrind
)

additional_ubuntu_packages=( \
  bcc
  bpfcc-tools
  ripgrep
)

case "${ID}" in
  debian)
    export DEBIAN_FRONTEND="noninteractive"

    pkg_list=("${common_debian_packages[@]}" selinux-utils)

    case "${VERSION_CODENAME}" in
      stretch)
        ;;

      *)
        pkg_list+=(ripgrep)
        ;;
    esac

    sudo -E apt-get install -y --no-install-recommends --allow-change-held-packages "${pkg_list[@]}"
    ;;

  ubuntu)
    export DEBIAN_FRONTEND="noninteractive"

    pkg_list=("${common_debian_packages[@]}")
    pkg_list+=("${additional_ubuntu_packages[@]}")

    sudo -E apt-get install -y --no-install-recommends --allow-change-held-packages "${pkg_list[@]}"
    ;;

  centos)
    pkg_list=( \
      perf
      redhat-lsb perl
      openssl ca-certificates
      openssh-server fuse-sshfs
      curl wget net-tools nmap-ncat bind-utils
      tmux vim-minimal
      python3-pip
    )

    sudo yum install -y "${pkg_list[@]}"
    sudo pip3 install bcrypt
    ;;
esac
