#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

distros_and_kernels=(
"centos 7"

"debian buster64"
"debian bullseye64"

"ubuntu bionic64"
"ubuntu focal64"
"ubuntu focal64 5.4.0-99-generic"
"ubuntu focal64 5.11.0-46-generic"
"ubuntu focal64 5.13.0-52-generic"
"ubuntu focal64 5.14.0-1004-oem"
# this is the kernel version that jammy uses as of November, 2022
"ubuntu focal64 5.15.0-52-generic"
"ubuntu jammy64"
)

