#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# For a docker image directory DIR ($1), checks if that version is not in docker. If so,
# touches FILENAME ($2). If FILENAME itself is missing, creates it.

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

DIR="$1"
FILENAME="$2"
IMAGE_TAG=$(${SCRIPTDIR}/get_tag.sh ${DIR})
EXISTING=$(docker images --filter "reference=${IMAGE_TAG}" -q)

# if the docker image doesn't exists, touch the file
if [ "${EXISTING}" == "" ]
then
    echo "No existing image ${IMAGE_TAG}. Touching ${FILENAME}."
    touch "${FILENAME}"
    exit 0
fi

# if `missing` is itself not on the filesystem, create it
# does the version file exist
if [ ! -f "${FILENAME}" ]
then
    echo "File ${FILENAME} does not exist when checking for existing image ${IMAGE_TAG}. Touching. $(pwd)"

    # doesn't exist, just create it.
    touch "${FILENAME}"

    exit 0
fi

#echo "${IMAGE_TAG} and ${FILENAME} exist. No change."
