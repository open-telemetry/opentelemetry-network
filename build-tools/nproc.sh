#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

if which nproc > /dev/null; then
  echo "$(((`nproc --all` + 1) / 2))"
elif which sysctl > /dev/null; then
  sysctl -n hw.physicalcpu
else
  # some default value - erring on the conservative side to avoid cache trashing
  echo 2
fi
