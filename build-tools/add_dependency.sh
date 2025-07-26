#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -e

if [ -z "$2" ]; then
  echo "usage: $0 dependency_name dependency_git_url"
  exit 1
fi

dep_name="$1"
dep_repo="$2"

echo "adding dependency $dep_name"
echo "  from repo $dep_repo"

mkdir "$dep_name"
sed "s/DEPENDENCY_NAME/$dep_name/g" \
  ".templates/dependency/Dockerfile" \
  > "$dep_name/Dockerfile"
git add "$dep_name/Dockerfile"

sed -i \
  -e "s/^#gen:dep-dir\$/build_directory($dep_name DEPENDS base)\n&/g" \
  -e "s/^\(build_directory(final DEPENDS base .*\))\$/\1 $dep_name)/g" \
  CMakeLists.txt
git add CMakeLists.txt

sed -i \
  -e "s/^#gen:dep-arg\$/ARG ${dep_name}_IMAGE_TAG\n&/g" \
  -e "s/^#gen:dep-from\$/FROM \\\$${dep_name}_IMAGE_TAG as build-${dep_name}\n&/g" \
  -e "s/^#gen:dep-copy\$/COPY --from=build-${dep_name} \\\$HOME\/install \\\$HOME\/install\n&/g" \
  final/Dockerfile
git add final/Dockerfile

git submodule add "$dep_repo" "$dep_name/$dep_name"

git commit -m "adding dependency $dep_name"

echo
echo "ACTION REQUIRED:"
echo "update file \`$dep_name/Dockerfile\` with proper build instructions then update commit with:"
echo
echo "  editor \"$dep_name/Dockerfile\" \\"
echo "    && git add \"$dep_name/Dockerfile\" \\"
echo "    && git commit --amend --no-edit"
