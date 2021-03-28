//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

package net.flowmill.render.extensions

import net.flowmill.render.render.Field
import static extension net.flowmill.render.extensions.FieldTypeExtensions.*

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
