// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.extensions

import net.flowmill.render.render.MetricField
import static extension net.flowmill.render.extensions.FieldTypeExtensions.*

class MetricFieldExtensions {

	static def cType(MetricField field) {
		// using -1 for non-array type
		return field.type.cType(-1);
	}
}
