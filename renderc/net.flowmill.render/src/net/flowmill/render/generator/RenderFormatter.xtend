// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import com.google.inject.Provider
import net.flowmill.render.render.Field
import net.flowmill.render.render.Message
import net.flowmill.render.render.AppRpcMap
import net.flowmill.render.render.ReferenceBindingValue
import org.eclipse.emf.ecore.EObject
import org.eclipse.xtext.formatting2.AbstractFormatter2
import org.eclipse.xtext.formatting2.IFormattableDocument
import org.eclipse.xtext.formatting2.IFormatter2

class RenderFormatter extends AbstractFormatter2 {

	static class Factory implements Provider<IFormatter2>{
		override RenderFormatter get() {
			return new RenderFormatter
		}
	}

	/**
	 * How to default-format the internal part of an EObject
	 */
	def void formatInternal(EObject p, extension IFormattableDocument doc) {
		p.regionFor.keyword('internal').surround[oneSpace]
		p.regionFor.keywords('-').forEach[prepend[setNewLines(1,1,2)]]
		interior(
			p.regionFor.keyword('{{'),
			p.regionFor.keyword('}}').prepend[setNewLines(1,1,2)],
			[indent]
		)
	}

	/**
	 * How to default-format the non-internal part of an EObject
	 */
	def void formatSingleBrace(EObject p, extension IFormattableDocument doc) {
		interior(
			p.regionFor.keyword('{').append[setNewLines(1,1,2)],
			p.regionFor.keyword('}'),
			[indent]
		)
		p.append[setNewLines(1,1,2)]
	}

	/**
	 * Format children recursively
	 */
	def void formatChildren(EObject p, extension IFormattableDocument doc) {
		for (pp : p.eContents)
			format(pp, doc)
	}

	/**
	 * Complete default recursive formatting
	 */
	def void defaultFormat(EObject p, extension IFormattableDocument doc) {
		formatSingleBrace(p, doc)
		formatInternal(p, doc)
		formatChildren(p, doc)
	}

	/**
	 * Apply the default formatting if there isn't a special case
	 */
	override dispatch void format(EObject p, extension IFormattableDocument doc) {
		defaultFormat(p, doc)
	}

	/******************
	 * Special cases
	 ******************/

	def dispatch void format(AppRpcMap p, extension IFormattableDocument doc) {
		/* do nothing */
	}

	def dispatch void format(ReferenceBindingValue p, extension IFormattableDocument doc) {
		/* do nothing */
	}

	def dispatch void format(Message p, extension IFormattableDocument doc)
	{
		p.regionFor.keywords('description', 'severity').forEach[prepend[setNewLines(1,1,2)]]
		defaultFormat(p, doc)
	}

	def dispatch void format(Field p, extension IFormattableDocument doc) {
		formatSingleBrace(p, doc)
		p.regionFor.keyword('{{').surround[oneSpace]
		p.regionFor.keyword('}}').append[setNewLines(1,1,2)]
		p.prepend[setNewLines(1,1,2)]
	}

}
