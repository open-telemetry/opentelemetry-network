// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import io.opentelemetry.render.render.MetricField
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.*

class MetricFieldExtensions {

	static def cType(MetricField field) {
		// using -1 for non-array type
		return field.type.cType(-1);
	}
}
