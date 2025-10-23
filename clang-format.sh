#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


CLANG_FORMAT="clang-format-19"
if ! command -v ${CLANG_FORMAT}
then
  echo "ERROR: requires ${CLANG_FORMAT}"
  exit 1
fi

RC=0
CMD="${CLANG_FORMAT} -Werror -i -style=file"
function format_file
{
  if ! ${CMD} $1
  then
    RC=1
  fi
}

# Check that C and C++ source files are properly clang-formatted
FILES=$(find ./geoip ./reducer ./test ./collector/kernel ./common ./tools \
	-type f                                                           \
	\( -name "*.c"                                                    \
	-o -name "*.cc"                                                   \
	-o -name "*.h"                                                    \
	-o -name "*.inl" \)                                               \
	-print)

for FILE in ${FILES}
do
  format_file ${FILE}
done

exit ${RC}
