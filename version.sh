#!/bin/sh
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set -e

# -----
# Major and minor versions influence customer vetting processes and level
# of perceived trust in an implementation -- please discuss before bumping these.
export FLOWMILL_MAJOR_VERSION='0'
export FLOWMILL_MINOR_VERSION='9'

# -----
# Build number is incremented automatically, so we can release directly from
# CI/CD, releasing the same exact binaries and containers that we already tested
# in dev/staging environements.

# COLLECTOR_BUILD_NUMBER_BASE is the commit-ish we count the build number from.
# to reset the build number, change this. Make sure this also happens together with
# bumping the major or minor versions, so there is no ambiguity with previously
# built versions.
# The commit hash must be a valid git hash in the flowmill-collector repository.
COLLECTOR_BUILD_NUMBER_BASE="3598385a6f5288ced0f7bbe9150837c4497d6bc8"
GIT_REVISION="HEAD"

# build number is the number of commits since the latest tag
FLOWMILL_COLLECTOR_BUILD_NUMBER="$(git rev-list --count "${COLLECTOR_BUILD_NUMBER_BASE}..${GIT_REVISION}")"
# adjust build number to keep the increasing order
FLOWMILL_COLLECTOR_BUILD_NUMBER="$(($FLOWMILL_COLLECTOR_BUILD_NUMBER + 4000))"
export FLOWMILL_COLLECTOR_BUILD_NUMBER
