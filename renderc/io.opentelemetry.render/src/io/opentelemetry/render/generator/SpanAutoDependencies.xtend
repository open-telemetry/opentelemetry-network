// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import java.util.Set
import java.util.Map
import java.util.List
import java.util.HashMap
import java.util.HashSet
import java.util.Vector
import java.util.LinkedList

import com.google.common.collect.Sets

import io.opentelemetry.render.render.Span
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.Reference
import io.opentelemetry.render.render.Definition
import io.opentelemetry.render.render.ReferenceBindingRoot
import io.opentelemetry.render.render.ReferenceBindingValue
import io.opentelemetry.render.render.ReferenceBindingRef

class SpanAutoDependencies {

  /**
   * All prerequisites used by auto references.
   */
  public final Set<Definition> prereqs

  /**
   * All non-dynamic prerequisites (fields, manual references) used by auto references.
   */
  public final Set<Definition> nonDynamicPrereqs

  /**
   * The set of prereqs for each auto reference.
   */
  public final Map<Reference, Set<Definition>> refPrereqs

  /**
   * The set of non-dynamic prereqs for each auto reference.
   */
  public final Map<Reference, Set<Definition>> refNonDynamicPrereqs

  /**
   * A valid ordering of all references according to dependencies.
   */
  public final List<Reference> computeOrder;

  new(Span span) {
    prereqs = new HashSet<Definition>()
    refPrereqs = new HashMap<Reference, Set<Definition>>()

    /* first, collect all the prereqs of auto/cached references */
    for (ref : span.definitions.filter(Reference).filter[isAuto || isIsCached]) {
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
      refPrereqs.put(ref, deps)
    }

    /* find a topological order of computing auto/cached references: */

    /* the graph of dependencies between references */
    val topoGraph = refPrereqs.mapValues[filter(Reference).filter[isAuto || isCached].toSet]

     /* in-degrees in the graph, used to find all sources */
    val inDegrees = topoGraph.mapValues[size]

    /* start with all sources in the original graph */
    val queue = new LinkedList<Reference>(inDegrees.filter[k,v | v == 0].keySet())

    val fullComputeOrder = new Vector<Reference>()

    while (!queue.empty) {
      /* dequeue */
      val ref = queue.poll()

      /* add to topological ordering */
      fullComputeOrder.add(ref)

      /* remove from graph: update inDegrees */
      for (v : topoGraph.get(ref)) {
        val newDegree = inDegrees.get(v) - 1

        inDegrees.put(v, newDegree)

        /* if v became a source, it can be queued */
        if (newDegree == 0) {
          queue.add(v)
        }
      }
    }

    /* check if there is a cycle */
    if (!(inDegrees.values.filter[degree | degree != 0].empty)) {
      throw new RuntimeException('''Dependencies in span "«span.name»" have a cycle;
        refs:«FOR ref: inDegrees.filter[k,v | v != 0].keySet()» «ref.name»«ENDFOR»''')
    }

    /* for each reference, we compute a closure on all its dependencies */
    for (ref : fullComputeOrder) {
      /* get direct prereqs */
      val deps = refPrereqs.get(ref)

      /* which direct prereqs have prereqs of their own */
      val otherReferences = deps.filter(Reference).filter[isAuto || isCached].toList

      /* add our prereqs' prereqs. because we're iterating in topological
       * order, this is sufficient to compute closure (the prereqs of
       * references earlier in the ordering are already closed)
       */
      for (other : otherReferences) {
        deps.addAll(refPrereqs.get(other))
      }

      refPrereqs.put(ref, deps)
    }

    /* can now union all prereqs of auto references to get all prereqs */
    for (deps : refPrereqs.filter[k,v | k.isAuto].values()) {
      prereqs.addAll(deps)
    }

    /* in the compute_order, we only need to compute auto references, or
     * cached references that an auto reference depends on */
    computeOrder = fullComputeOrder.filter[ref | ref.isAuto || prereqs.contains(ref)].toList

    nonDynamicPrereqs = prereqs.filter[defn |
        (defn instanceof Field) ||
        ( (defn instanceof Reference) &&
         !((defn as Reference).isAuto || (defn as Reference).isCached))
    ].toSet

    refNonDynamicPrereqs = refPrereqs.mapValues[v | Sets.intersection(v, nonDynamicPrereqs)]
  }
}
