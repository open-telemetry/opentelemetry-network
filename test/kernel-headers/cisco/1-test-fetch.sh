#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe
vagrant ssh -- -- sudo rm -rf /var/cache/ebpf_net/kernel-headers || true
vagrant ssh -- -R "5000:localhost:5000" -- ./server.sh
vagrant ssh -- -R "5000:localhost:5000" -- ./agent.sh
