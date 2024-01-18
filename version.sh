#!/bin/sh
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set -e

# -----
# Major and minor versions influence customer vetting processes and level
# of perceived trust in an implementation -- please discuss before bumping these.
export EBPF_NET_MAJOR_VERSION='0'
export EBPF_NET_MINOR_VERSION='10'
export EBPF_NET_PATCH_VERSION='2'

# -----
# Build number is incremented automatically, so we can release directly from
# CI/CD, releasing the same exact binaries and containers that we already tested
# in dev/staging environements.

# COLLECTOR_BUILD_NUMBER_BASE is the commit-ish we count the build number from.
# to reset the build number, change this. Make sure this also happens together with
# bumping the major or minor versions, so there is no ambiguity with previously
# built versions.
# The commit hash must be a valid git hash in the opentelemetry-ebpf repository.
COLLECTOR_BUILD_NUMBER_BASE="3598385a6f5288ced0f7bbe9150837c4497d6bc8"
GIT_REVISION="HEAD"

# build number is the number of commits since the latest tag
EBPF_NET_COLLECTOR_BUILD_NUMBER="$(git rev-list --count "${COLLECTOR_BUILD_NUMBER_BASE}..${GIT_REVISION}")"
# adjust build number to keep the increasing order
EBPF_NET_COLLECTOR_BUILD_NUMBER="$(($EBPF_NET_COLLECTOR_BUILD_NUMBER + 4000))"
export EBPF_NET_COLLECTOR_BUILD_NUMBER
