#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

docker run --detach --rm \
  --expose 8000 \
  --expose 7000 \
  localhost:5000/flowtune-server \
    --port 8000 \
    --prom 0.0.0.0:7000 \
    --partitions-per-shard 1 \
    --enable-aws-enrichment \
    --log-console \
    --debug
