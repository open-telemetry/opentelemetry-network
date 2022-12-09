#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

docker run -d -p 5000:5000 --restart always --name local-docker-registry --env "REGISTRY_STORAGE_DELETE_ENABLED=true" registry:latest
