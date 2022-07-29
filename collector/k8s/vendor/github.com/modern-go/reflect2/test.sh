#!/usr/bin/env bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set -e
echo "" > coverage.txt

for d in $(go list github.com/modern-go/reflect2-tests/... | grep -v vendor); do
    go test -coverprofile=profile.out -coverpkg=github.com/modern-go/reflect2 $d
    if [ -f profile.out ]; then
        cat profile.out >> coverage.txt
        rm profile.out
    fi
done
