// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import net.flowmill.render.render.Span
import net.flowmill.render.render.Definition
import java.util.Set
import java.util.Map
import net.flowmill.render.render.Reference
import java.util.List
import java.util.HashMap
import java.util.HashSet
import java.util.Vector
import net.flowmill.render.render.ReferenceBindingRoot
import net.flowmill.render.render.ReferenceBindingValue
import net.flowmill.render.render.ReferenceBindingRef
import java.util.LinkedList
import net.flowmill.render.render.Field
import com.google.common.collect.Sets

class SpanAutoDependencies {
	/**
	 * all prerequisites used by auto references
	 */
	public Set<Definition> prereqs

	/**
	 * all non_dynamic prerequisites (fields, manual references) used by auto
	 *   references
	 */
	public Set<Definition> non_dynamic_prereqs

	/**
	 * The set of prereqs for each auto reference
	 */
	public Map<Reference, Set<Definition>> ref_prereqs

	/**
	 * The set of non-dynamic prereqs for each auto reference
	 */
	public Map<Reference, Set<Definition>> ref_non_dynamic_prereqs

	/**
	 * A valid ordering of all references according to dependencies
	 */
	public List<Reference> compute_order;

	new(Span span) {
		prereqs = new HashSet<Definition>()
		ref_prereqs = new HashMap<Reference, Set<Definition>>()
		compute_order = new Vector<Reference>()

		/* first, collect all the prereqs of auto/cached references */
		for (ref : span.definitions.filter(Reference)
			                       .filter[isAuto || isIsCached]) {
			val deps = new HashSet<Definition>()
			/* for each binding, reach the root entity */
			for (binding : ref.bindings) {
				var ReferenceBindingRef traverse = binding.value
				/* we want the ReferenceBindingRoot */
				while (traverse instanceof ReferenceBindingValue) {
					traverse = traverse.ref
				}
				deps.add((traverse as ReferenceBindingRoot).entity)
			}

			/* set the ref's prereqs */
			ref_prereqs.put(ref, deps)
		}

		/* find a topological order of computing auto/cached references: */

		/* the graph of dependencies between references */
		val topo_graph = ref_prereqs.mapValues[filter(Reference)
 				   	                  			.filter[isAuto || isCached]
 				   	                  			.toSet]

 		/* in-degrees in the graph, used to find all sources */
		val in_degrees = topo_graph.mapValues[size]

		/* start with all sources in the original graph */
		val queue = new LinkedList<Reference>(in_degrees.filter[k,v | v == 0].keySet())

		while (!queue.empty) {
			/* dequeue */
			val ref = queue.poll()

		 	/* add to topological ordering */
		 	compute_order.add(ref)

		 	/* remove from graph: update in_degrees */
		 	for (v : topo_graph.get(ref)) {
		 		val new_degree = in_degrees.get(v) - 1
		 		in_degrees.put(v, new_degree)

		 		/* if v became a source, it can be queued */
		 		if (new_degree == 0) {
		 			queue.add(v)
		 		}
		 	}
		}

		/* check if there is a cycle */
		if (!(in_degrees.values.filter[degree | degree != 0].empty)) {
		 	throw new RuntimeException('''dependencies in span "«span.name»" have a cycle.
		 	  refs:«FOR ref: in_degrees.filter[k,v | v != 0].keySet()» «ref.name»«ENDFOR»''')
		}

		/* for each reference, we compute a closure on all its dependencies */
		for (ref : compute_order) {
			/* get direct prereqs */
			val deps = ref_prereqs.get(ref)

			/* which direct prereqs have prereqs of their own */
			val other_references = deps.filter(Reference).filter[isAuto || isCached].toList

			/* add our prereqs' prereqs. because we're iterating in topological
			 * order, this is sufficient to compute closure (the prereqs of
			 * references earlier in the ordering are already closed)
			 */
			for (other : other_references)
				deps.addAll(ref_prereqs.get(other))

			ref_prereqs.put(ref, deps)
		}

		/* can now union all prereqs of auto references to get all prereqs */
		for (deps : ref_prereqs.filter[k,v | k.isAuto].values()) {
			prereqs.addAll(deps)
		}

		/* in the compute_order, we only need to compute auto references, or
		 * cached references that an auto reference depends on */
		compute_order = compute_order.filter[ref | ref.isAuto || prereqs.contains(ref)].toList

		non_dynamic_prereqs = prereqs.filter[defn |
			   (defn instanceof Field)
			|| (   (defn instanceof Reference)
				&& !(   (defn as Reference).isAuto
					 || (defn as Reference).isCached))].toSet

		ref_non_dynamic_prereqs =
			ref_prereqs.mapValues[v | Sets.intersection(v, non_dynamic_prereqs)]
	}
}
