#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

# centos
./bootstrap.sh centos 7
./bootstrap.sh centos 7 3.10.0-1127.el7.x86_64

# debian
./bootstrap.sh debian buster64
./bootstrap.sh debian bullseye64

# ubuntu
./bootstrap.sh ubuntu disco64
./bootstrap.sh ubuntu bionic64
./bootstrap.sh ubuntu focal64
./bootstrap.sh ubuntu focal64 5.4.0-99-generic
./bootstrap.sh ubuntu focal64 5.8.18-050818-generic
./bootstrap.sh ubuntu focal64 5.11.0-46-generic
./bootstrap.sh ubuntu focal64 5.13.0-52-generic
./bootstrap.sh ubuntu focal64 5.14.0-1004-oem # eBPF errors as of November, 2022
./bootstrap.sh ubuntu focal64 5.14.0-1045-oem # eBPF errors as of November, 2022
./bootstrap.sh ubuntu focal64 5.15.0-33-generic # eBPF errors as of November, 2022
# this is the kernel version that jammy uses as of November, 2022
./bootstrap.sh ubuntu focal64 5.15.0-52-generic # eBPF errors as of November, 2022
./bootstrap.sh ubuntu jammy64 # eBPF errors as of November, 2022
