#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# This script gets the latest git modification hash for a specific directory (DIR=$1).
# If docker doesn't have an image named ${BENV_PREFIX}-${DIR}:${VERSION_HASH},
#  builds that image in DIR.

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

DIR="$1"
shift 1

IMAGE_TAG=$(${SCRIPTDIR}/get_tag.sh ${DIR})
EXISTING=$(docker images --filter "reference=${IMAGE_TAG}" -q)

# if the docker image already exists, we're done
if [ "${EXISTING}" != "" ]
then
    echo ${IMAGE_TAG}: exists
    exit 0
fi

echo ${IMAGE_TAG}: does not exist, building
docker build ${DIR} -t ${IMAGE_TAG} $@
