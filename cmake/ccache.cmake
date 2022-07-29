# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# use ccache if available. thanks to http://stackoverflow.com/a/24305849
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)
