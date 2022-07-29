// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.extensions

import net.flowmill.render.render.Message
import net.flowmill.render.render.MessageType
import net.flowmill.render.render.Span

import static extension net.flowmill.render.extensions.FieldExtensions.arraySuffix
import static extension net.flowmill.render.extensions.FieldTypeExtensions.parsedCType

class MessageExtensions {
	static def _prepend_comma_if_not_empty(String s) {
		if (s == "")
			s
		else
			", " + s
	}

	static def prototype(Message msg) {
		val fields = msg.fields.sortBy[id]
		val strs = fields.map[type.isShortString
			? '''const char «name»[«type.size»]«arraySuffix»'''
			: '''const «type.parsedCType» «name»«arraySuffix»''']
		strs.join(", ")
	}

	static def commaPrototype(Message msg) {
		_prepend_comma_if_not_empty(msg.prototype)
	}

	static def callPrototype(Message msg) {
		msg.fields.sortBy[id].map[name].join(", ")
	}

	static def commaCallPrototype(Message msg) {
		_prepend_comma_if_not_empty(msg.callPrototype)
	}

	static def norefPrototype(Message msg) {
		val fields = msg.fields.filter[field | field !== msg.reference_field].sortBy[id]
		val strs = fields.map['''const «type.parsedCType» «name»«arraySuffix»''']
		strs.join(", ")
	}

	static def norefCommaPrototype(Message msg) {
		val fields = msg.fields.filter[field | field !== msg.reference_field].sortBy[id]
		val strs = fields.map['''const «type.parsedCType» «name»«arraySuffix»''']
		val str = strs.join(", ")
		_prepend_comma_if_not_empty(str)
	}

	static def norefCommaCallPrototype(Message msg) {
		val fields = msg.fields.filter[field | field !== msg.reference_field].sortBy[id]
		val strs = fields.map[type.isShortString ? '''«name».data()''' : name]
		val str = strs.join(", ")
		_prepend_comma_if_not_empty(str)
	}

	static def span(Message msg) {
		msg.eContainer as Span
	}

	// Names of errors that handling of this message can trigger.
	//
	static def errors(Message msg) {
		if (msg.type == MessageType.START) {
			#{"span_alloc_failed", "span_insert_failed", "span_pool_full", "duplicate_ref"}
		} else if (msg.type == MessageType.END) {
			#{"span_find_failed", "span_erase_failed"}
		} else if (!msg.span.isSingleton) {
			#{"span_find_failed"}
		} else {
			#{}
		}
	}
}
