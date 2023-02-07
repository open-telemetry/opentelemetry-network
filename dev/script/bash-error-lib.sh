#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -Eeo pipefail

trap 'catch $? $LINENO' ERR
catch() {
  echo "Error $1 occurred at $0 line $2"
  exit "$1"
}
