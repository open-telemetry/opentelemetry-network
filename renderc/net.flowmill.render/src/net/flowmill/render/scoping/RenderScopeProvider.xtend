// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.scoping

import net.flowmill.render.render.Span
import net.flowmill.render.render.Field
import net.flowmill.render.render.Reference
import net.flowmill.render.render.ReferenceBinding
import net.flowmill.render.render.ReferenceBindingRoot
import net.flowmill.render.render.ReferenceBindingValue
import net.flowmill.render.render.RenderPackage
import org.eclipse.emf.ecore.EObject
import org.eclipse.emf.ecore.EReference
import org.eclipse.xtext.scoping.IScope
import org.eclipse.xtext.scoping.Scopes
import org.eclipse.xtext.scoping.impl.AbstractDeclarativeScopeProvider
import net.flowmill.render.render.AggregationUpdate

/**
 * This class contains custom scoping description.
 *
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#scoping
 * on how and when to use it.
 */
class RenderScopeProvider extends AbstractDeclarativeScopeProvider {

	/**
	 * ReferenceBinding::value can follow references to other spans
	 *
	 * follows pattern from:
	 *   https://christiandietrich.wordpress.com/2013/05/18/xtext-and-dot-expressions/
	 */
	def IScope scope_ReferenceBindingValue_tail(ReferenceBindingValue exp,
		EReference reference)
	{
		val head = exp.ref;
		switch (head) {
			ReferenceBindingRoot: {
				val entity = head.entity;
				switch(entity) {
					Field :		IScope::NULLSCOPE
					Reference:	Scopes.scopeFor(entity.target.definitions)
					default:	IScope::NULLSCOPE
				}
			}
			ReferenceBindingValue: {
				val tail = head.tail
				switch (tail) {
					Field: 		IScope::NULLSCOPE
					Reference: 	Scopes.scopeFor(tail.target.definitions)
					default:	IScope::NULLSCOPE
				}
			}
		}
	}

	def IScope scope_AggregationUpdate_agg(AggregationUpdate exp,
		EReference reference)
	{
		Scopes.scopeFor(exp.ref.target.aggs);
	}

	def IScope scope_Span_remoteSpan(Span exp, EReference reference)
	{
		Scopes.scopeFor(exp.remoteApp.spans);
	}

	override getScope(EObject context, EReference reference) {
//		println("getScope context " + context + " ref " + reference);

		/* ReferenceBinding::key should be scoped inside Reference::target */
		if (reference == RenderPackage.Literals.REFERENCE_BINDING__KEY)
		{
			/*
			 * we can be called from Reference or ReferenceBinding context.
			 * In either case, find the Reference context.
			 */
			val ref = switch(context) {
				Reference: context
				ReferenceBinding: context.eContainer() as Reference
			}

			/* get the Reference::target (which is an Index) */
			val span = ref.target
			val index = span.index;

			/* get all the Index::keys (which are references to Definition) */
			return Scopes.scopeFor(index.keys);
		}

		/* if we didn't override, call the default implementation */
		return super.getScope(context, reference);
	}
}
