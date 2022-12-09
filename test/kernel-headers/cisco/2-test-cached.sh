#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe
vagrant ssh -- -R "5000:localhost:5000" -- ./agent.sh
