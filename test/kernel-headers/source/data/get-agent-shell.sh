#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

container_id_file="/tmp/container.id"
export container_id="$(cat "${container_id_file}")"

docker exec -it "${container_id}" bash
