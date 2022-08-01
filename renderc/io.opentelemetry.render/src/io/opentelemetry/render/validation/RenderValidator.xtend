// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.validation

import org.eclipse.xtext.validation.Check
import net.flowmill.render.render.Field
import net.flowmill.render.render.FieldTypeEnum
import net.flowmill.render.render.Reference
import net.flowmill.render.render.Span
import net.flowmill.render.render.RenderPackage
import net.flowmill.render.render.Message
import net.flowmill.render.render.MessageType
import net.flowmill.render.render.RpcIdRange
import static net.flowmill.render.generator.AppPacker.rpcIdRangeMin
import static net.flowmill.render.generator.AppPacker.rpcIdRangeMax
import static extension net.flowmill.render.extensions.MessageExtensions.span

/**
 * This class contains custom validation rules.
 *
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#validation
 */
class RenderValidator extends AbstractRenderValidator {

	@Check
	def void checkSpanDoesNotContainVariableStrings(Field field) {
		if (field.type.enum_type != FieldTypeEnum.STRING)
			return

		var parent = field.eContainer()
		switch (parent) {
			Span:
				error('Span should not contain variable-length strings',
					RenderPackage.Literals.FIELD__TYPE
				)
		}
	}

	@Check
	def void checkSingletonSpanDoesNotContainStartOrEnd(Message m) {
		switch (m.type) {
			case MessageType.START,
			case MessageType.END:
			/* need to check the containing span is not a singleton */
				if ((m.eContainer() as Span).isSingleton)
					error('Singleton Span should not contain start or end messages',
						RenderPackage.Literals.MESSAGE__TYPE
					)
			default:
				return
		}
	}

	@Check
	def void checkMessageHasReferenceField(Message msg) {
		if (msg.type == MessageType.MSG) {
			// messages declared with the 'msg' syntatic sugar will be
			// added a special reference field automatically
			return
		}

		if (!msg.span.isSingleton && (msg.referenceEmbedded == false)) {
			error('''Message is missing a reference field''',
				RenderPackage.Literals.MESSAGE__TYPE)
		}
	}

	@Check
	def void checkProxySpanKeyIsSupersetOfRemoteSpanKey(Span span) {
		if (span.isProxy) {
			if (span.remoteSpan.index === null) {
				// remote span is not index-allocated
				return
			}

			if (span.index === null) {
				error('''Proxy span index key must be a superset of the remote span's key''',
					RenderPackage.Literals.SPAN__INDEX)
			}

			val keys = span.index.keys.filter(Field)

			for (remoteKey : span.remoteSpan.index.keys.filter(Field)) {
				val localKey = keys.findFirst[name == remoteKey.name]

				if (localKey === null) {
					error('Proxy span index key is missing a field: ' + remoteKey.name,
						RenderPackage.Literals.SPAN__INDEX)
				}

				if (localKey.type.enum_type != remoteKey.type.enum_type) {
					error('Proxy span index key field is of wrong type: ' + localKey.name,
						RenderPackage.Literals.SPAN__INDEX)
				}
			}
		}
	}

	@Check
	def void checkShardingKeyIsSubsetOfIndexKey(Span span) {
		if (span.sharding === null) {
			return
		}

		if (span.index === null) {
			// non-indexed span
			return
		}

		var indexKeyFields = span.index.keys.filter(Field).toSet()

		for (field : span.sharding.keys) {
			if (indexKeyFields.contains(field) == false) {
				error("Sharding key field '" + field.name + "' must be part of the index key",
					RenderPackage.Literals.SPAN__REMOTE_SPAN)
			}
		}
	}

	@Check
	def void checkRemoteSpanKeyHasNoReferences(Span proxySpan) {
		// NOTE: given a span, there is no way to check whether it's a target
		//       of some proxy span, so we have to check remote spans of all
		//       proxy span
		var span = proxySpan.remoteSpan

		if (span === null) {
			return
		}

		var refs = span.index.keys.filter(Reference)

		if (refs.size > 0) {
			error("Span '" + span.name + "' is a target of a proxy so it can't have reference fields in its key",
				RenderPackage.Literals.SPAN__REMOTE_SPAN)
		}
	}

	@Check
	def void checkRpcIdRange(RpcIdRange range) {
		if (range.start < rpcIdRangeMin) {
			error('''Invalid RPC ID range (start < «rpcIdRangeMin»)''',
				RenderPackage.Literals.RPC_ID_RANGE__START)
		}

		if (range.start > rpcIdRangeMax) {
			error('''Invalid RPC ID range (start > «rpcIdRangeMax»)''',
				RenderPackage.Literals.RPC_ID_RANGE__START)
		}

		if (range.hasEnd) {
			if (range.end < range.start) {
				error("Invalid RPC ID range (end < start)",
					RenderPackage.Literals.RPC_ID_RANGE__END)
			}

			if (range.end > rpcIdRangeMax) {
				error('''Invalid RPC ID range (end > «rpcIdRangeMax»)''',
					RenderPackage.Literals.RPC_ID_RANGE__END)
			}
		}
	}
}
