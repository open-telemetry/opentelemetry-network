// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import com.google.common.collect.ImmutableList
import java.util.Collections
import java.util.HashSet
import java.util.LinkedList
import java.util.List
import java.util.Map
import java.util.Set
import java.util.TreeMap
import java.util.stream.LongStream
import net.flowmill.render.render.App

class PerfectHash {

	static def bit_length(int x) {
		Integer.SIZE - Integer.numberOfLeadingZeros(x)
	}

	int n_keys

	int n_bits
	int g_size
	int g_shift

	int hash_bits
	int hash_shift
	int hash_mask

	long hash_seed
	long multiplier

	String g_type
	String hash_func_name

	int[] g_arr

	static def isUnique(List<Long> lst) {
		lst.toSet.size == lst.size
	}

	def Map<Long, List<Long>> findSeed(List<Integer> int_keys)
	{
		val KNUTH_A = 2654435761L

	    // the Knuth multiplier plus a bunch of random 32 bit primes
	    val MULTIPLIERS = ImmutableList.of(KNUTH_A, 3692456357L, 2257312319L, 3145181107L, 2857830571L,
	                   3361085821L, 4185606979L, 2982027877L)

	    for (seed : LongStream.range(0, 1).toArray) {
	        for (mul : MULTIPLIERS) {
	            // hash the keys
	            val longMask = (1L << 32) - 1
	            val mapped_keys = int_keys.map[k | (k.longValue.bitwiseXor(seed) * mul).bitwiseAnd(longMask)]

	            // arrange keys into bins
				val bins = mapped_keys.groupBy[k | k >> g_shift]

	            // make sure there are no collisions in (k >> hash_shift) & hash_mask
	            //  within each bin
	            val found_seed = bins.values.forall[isUnique(map[k | (k >> hash_shift).bitwiseAnd(hash_mask)])]

	            if (found_seed) {
	            	hash_seed = seed
	            	multiplier = mul
	            	return bins
	            }
            }
        }

        throw new RuntimeException("Could not find seed - check for duplicate RPC IDs or exhausted namespace allocations")
	}

	/**
	 * Finds the g value for the bin, given already assigned slots
	 */
	def find_gval(List<Long> bin, Set<Long> already_assigned_slots,
		boolean verbose
	) {
        // search for proper value of g
        for (var gval = 0; gval <= hash_mask; gval++) {
        	val gv = gval
            // where bin keys would fall
            val slots = bin.map[k |
            	((k >> hash_shift) + gv).bitwiseAnd(hash_mask)
            ].toSet

            // sanity check
            if(slots.size != bin.size)
            	throw new RuntimeException("find_gval: slots not unique")

            // would the keys fit nicely?
            if (Collections.disjoint(slots,already_assigned_slots)) {
		        // update the taken slots with those occupied by the bin
		        already_assigned_slots.addAll(slots)
		        if (verbose)
		        	println("allocated slots" + slots + " with g " + gv
		        		+ " (k >> g_shift) " + (bin.get(0) >> g_shift)
		        	)

            	return gval
            }
        }

        throw new RuntimeException("could not find g value!")
	}

	new(App app, boolean verbose) {
		val int_keys = app.spans.flatMap[messages.map[wire_msg.rpc_id]].toList

		hash_func_name = app.name + "_hash"

		n_keys = int_keys.size()

	    // how many bits for g
	    n_bits = bit_length((n_keys + 4) / 5)
	    g_size = 1 << n_bits
	    g_shift = if (n_bits == 0) 0 else 32 - n_bits

	    // how to compute hash destination
	    hash_bits = bit_length((n_keys* 1.2 + 1).intValue)
	    hash_shift = 32 - n_bits - hash_bits
	    hash_mask = (1 << hash_bits) - 1

	    val bins_map = findSeed(int_keys)

	    // sort bins by size
	    val bins = bins_map.values.sortBy[size].reverse

	    // sanity check
	    if ((n_keys > 0) && (bins.map[size].reduce[p1, p2| p1 + p2] != int_keys.size))
	    	throw new RuntimeException("size sanity check failed!")

	    // map bins onto the key space
	    val g = new TreeMap<Integer, Integer>
	    val already_assigned_slots = new HashSet<Long>

	    for (bin : bins.filter[size > 1]) {
	        if (g.containsKey((bin.get(0) >> g_shift).intValue))
	        	throw new RuntimeException("assertion failed")

			val gval = find_gval(bin, already_assigned_slots, verbose)
			g.put((bin.get(0) >> g_shift).intValue, gval)
		}

	    // now assign all bins with only one value
	    // we prefer low slots
	    val free_slots = new LinkedList<Integer>
	    for (var i = 0; i <= hash_mask; i++) {
	    	if (!already_assigned_slots.contains(i.longValue))
	    		free_slots.add(i)
    	}

	    for (bin : bins.filter[size == 1]) {
	        val slot = free_slots.pop()
	        val elem = bin.get(0)
	        g.put((elem >> g_shift).intValue,
	        	(slot - (elem >> hash_shift) + hash_mask + 1)
	        		.bitwiseAnd(hash_mask).intValue)

	        if (verbose)
	        	println("allocated slot " + slot + " to " + (elem >> g_shift)
	        		+ " elem " + elem)
	    }
	    if (verbose) {
	    	println("num length-1 bins: " + bins.filter[size==1].size)
	    	println("g: " + g)
    	}

	    // sanity check: do we have enough g values?
	    if (g.size != bins.size)
	    	throw new RuntimeException("somehow didn't compute enough g values")

	    // map all values and make sure there are no repetitions
	    val locs = bins.flatten.map[k |
	    	((k >> hash_shift) + g.get((k >> g_shift).intValue)).bitwiseAnd(hash_mask)
		].toSet
	    if (verbose)
	    	println("len(locs) " + locs.size + " len(keys) " + int_keys.size)

	    if (locs.size != int_keys.size)
	    	throw new RuntimeException("somehow locs doesn't contain enough vals")

	    if (hash_mask + 1 <= 256)
	        g_type = "u8"
	    else if (hash_mask + 1 <= (1 << 16))
	        g_type = "u16"
	    else
	        g_type = "u32"

	    g_arr = newIntArrayOfSize(g_size)
	    for (var i = 0; i < g_size; i++) {
	    	g_arr.set(i, g.getOrDefault(i, 0))
	    }
	}

	def generateC() {
		'''
		#include <platform/platform.h>

		«g_type» «hash_func_name»_g_array[] = {
		    «g_arr.map[toString].join(",")»
		};
		'''
	}

	def generateH() {
		'''
		#pragma once

		#include <platform/platform.h>

		/**
		 * «hash_func_name»
		 *
		 * g_type: «g_type»
		 * g_size: «g_size»
		 * g_shift: «g_shift»
		 * hash_shift: «hash_shift»
		 * hash_mask: «hash_mask»
		 * n_keys: «n_keys»
		 * multiplier: «multiplier»
		 * hash_seed: «hash_seed»
		 */

		extern «g_type» «hash_func_name»_g_array[];

		#define «hash_func_name.toUpperCase»_SIZE    «hash_mask + 1»

		static inline u32 «hash_func_name»(u32 rpc_id) {
		    u32 k = (rpc_id ^ «hash_seed») * «multiplier»;
		    return ((k >> «hash_shift») + «hash_func_name»_g_array[k >> «g_shift»]) & «hash_mask»;
		}
		'''
	}

}
