#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# This script gets a tag for the given directory (DIR=$1) from the latest git modification.
# The image+tag will be ${BENV_PREFIX}-${DIR}:${VERSION_HASH}

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

DIR="$1"
BENV_PREFIX="${DOCKER_TAG_PREFIX}benv"

VERSION_HASH=$(git log -1 --format=%h $SCRIPTDIR/${DIR})

IMAGE_TAG="${BENV_PREFIX}-${DIR}:${VERSION_HASH}"

echo ${IMAGE_TAG}
