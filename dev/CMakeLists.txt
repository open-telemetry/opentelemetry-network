
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(script)

lint_shell_script_bundle(
  dev-scripts
  SOURCES
    benv-list.sh
    benv-run.sh
)
