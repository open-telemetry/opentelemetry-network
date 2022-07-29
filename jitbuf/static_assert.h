/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#define JB_STATIC_ASSERT(name, predicate) typedef char _jitbuf_static_assert_##name[2 * !!(predicate)-1];
