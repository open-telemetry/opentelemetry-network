// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import com.google.common.base.CaseFormat

class UtilityExtensions {
	static def toCamelCase(String s) {
	    CaseFormat.LOWER_UNDERSCORE.to(CaseFormat.UPPER_CAMEL, s)
	}

}
