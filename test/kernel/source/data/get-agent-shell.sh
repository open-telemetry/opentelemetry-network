#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

container_name="test-kernel-collector"
docker exec -it "${container_name}" bash
