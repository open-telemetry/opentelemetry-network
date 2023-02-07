#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe
update-ca-certificates
. .env | sort
/srv/entrypoint.sh "$@"
