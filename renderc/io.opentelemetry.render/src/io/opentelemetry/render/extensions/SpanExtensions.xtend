// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import io.opentelemetry.render.render.Span
import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.MessageType

import static extension io.opentelemetry.render.extensions.UtilityExtensions.toCamelCase

class SpanExtensions {

	static def getBaseClassName(Span span) {
	    toCamelCase(span.name) + "SpanBase"
	}
	static def getClassTypeName(Span span) {
	    span.name + "_type"
	}
	static def getInstanceName(Span span) {
	    span.name + "__instance"
	}
	static def fixedHashName(Span span) {
	    span.name + "__hash"
	}
	static def fixedHashTypeName(Span span) {
	    span.name + "__hash_t"
	}
	static def fixedHashHasherName(Span span) {
	    span.name + "__hasher_t"
	}

	static def proxyStartMessage(Span span) {
		val msg = span.remoteSpan.messages.findFirst[(type == MessageType.START) && referenceEmbedded]
		if (msg === null) {
			throw new RuntimeException("proxy: no viable start message: " + span.remoteSpan)
		}
		return msg
	}

	static def proxyEndMessage(Span span) {
		val msg = span.remoteSpan.messages.findFirst[(type == MessageType.END) && (fields.length == 1) && referenceEmbedded]
		if (msg === null) {
			throw new RuntimeException("proxy: no viable end message: " + span.remoteSpan)
		}
		return msg
	}

	static def proxyLogMessages(Span span) {
		span.remoteSpan.messages.filter[((type == MessageType.LOG) || (type == MessageType.MSG)) && referenceEmbedded]
	}

	static def app(Span span) {
		span.eContainer as App
	}

	static def referenceType(Span span) {
		val message_with_ref = span.messages.findFirst[referenceEmbedded]
		if (message_with_ref === null) {
			throw new RuntimeException("referenceType(span): span does not have any messages with reference_field")
		}
		message_with_ref.reference_field.type
	}

	static def pool_size(Span span) {
		if (span.pool_size_ > 0) {
			return span.pool_size_
		} else {
			return 4096;
		}
	}

	static def conn_hash(Span span) {
		if (span.conn_hash_) {
			return span.conn_hash_
		}

		return (span.messages.length > 0) && !span.isSingleton
	}
}
