// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import io.opentelemetry.render.render.Field
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.*

class FieldExtensions {

	/**
	 * Get field size, taking array size into account
	 */
	static def size(Field field, boolean packedStrings) {
		// num_elems is array_size, or 1 if not an array
		val num_elems = if (field.isArray) field.array_size else 1
		return field.type.size(packedStrings) * num_elems
	}

	/**
	 * Returns the "[size]" string for array fields, otherwise ""
	 */
	static def arraySuffix(Field field) {
		if (field.isArray)
			'''[«field.array_size»]'''
		else
			""
	}

	static def cType(Field field) {
		return field.type.cType(field.isIsArray ? field.array_size : -1);
	}
}
