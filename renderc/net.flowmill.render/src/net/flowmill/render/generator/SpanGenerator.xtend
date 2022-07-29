// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import net.flowmill.render.render.AggregationMethod
import net.flowmill.render.render.Field
import net.flowmill.render.render.Metric
import net.flowmill.render.render.Reference
import net.flowmill.render.render.ReferenceBindingRef
import net.flowmill.render.render.ReferenceBindingRoot
import net.flowmill.render.render.ReferenceBindingValue
import net.flowmill.render.render.Span
import net.flowmill.render.render.App
import net.flowmill.render.render.Aggregation
import net.flowmill.render.render.Message;
import org.eclipse.emf.ecore.resource.Resource
import net.flowmill.render.render.FieldTypeEnum
import java.util.Arrays

import static net.flowmill.render.generator.AppPacker.pulseMessageName

import static extension net.flowmill.render.extensions.AppExtensions.*
import static extension net.flowmill.render.extensions.FieldExtensions.*
import static extension net.flowmill.render.extensions.SpanExtensions.*
import static extension net.flowmill.render.extensions.MessageExtensions.*
import static extension net.flowmill.render.extensions.XPackedMessageExtensions.*

/**
 * Generates code from your model files on save.
 *
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#code-generation
 */
class SpanGenerator {

	static def generatedCodeWarning() {
		'''
		/**********************************************
		 * !!! render-generated code, do not modify !!!
		 **********************************************/

		'''
	}

	/**
	 * Generates metric update code for aggregation node
	 *
	 * |agg|: metric aggregator
	 * |store_name|: the name of MetricStore object
	 * |loc|: location
	 * |t|: timestamp of the metric
	 * |metric|: the metric to be merged into MetricStore
	 * |is_root|: whether the update is for root.
	 */
	static def generateMetricUpdate(Aggregation agg, String store_name, String loc, String t, String metric, boolean is_root) {
		'''
		auto it = «store_name».lookup(«loc», «t», true);
		if (it.first == false) {
			/* just enqueued, assign stats */
			«IF is_root»
				«FOR field : agg.type.fields»
					«IF field.method == AggregationMethod::TDIGEST»
						it.second.«field.name».add(«metric».«field.name»);
					«ELSE»
						it.second.m.«field.name» = «metric».«field.name»;
					«ENDIF»
				«ENDFOR»
			«ELSE»
				it.second = «metric»;
			«ENDIF»
			/* also increase the reference count */
			map[«loc»].__refcount++;
		} else {
			«IF is_root»
				«FOR field : agg.type.fields»
					«IF field.method == AggregationMethod::TDIGEST»
						it.second.«field.name».add(«metric».«field.name»);
					«ELSE»
						it.second.m.«field.name» += «metric».«field.name»;
					«ENDIF»
				«ENDFOR»
			«ELSE»
				«FOR field : agg.type.fields»
					«IF field.method == AggregationMethod::TDIGEST»
						it.second.«field.name».merge(«metric».«field.name»);
					«ELSE»
						it.second.«field.name» += «metric».«field.name»;
					«ENDIF»
				«ENDFOR»
			«ENDIF»
		}
		'''
	}

	/***************************************************************************
	 * Generate _foreach function for metric stores in an aggregation node.
	 *
	 * |agg|: Aggregation struct
	 * |span_name|: name of current span.
	 * |is_rollup|: whether this is for time-based roll-up
	 * |rollup_count|: how many intervals is the roll-up
	 **************************************************************************/
	static def generateMetricForeach(Aggregation agg, String span_name, boolean is_rollup, int rollup_count) {
		var store_name = agg.name;
		if (is_rollup) {
			store_name = store_name + "_" + rollup_count;
		}

		'''
		template<class FUNCTOR>
		void «span_name»::«store_name»_foreach(u64 t, FUNCTOR &&f)
		{
			«IF is_rollup»
				constexpr u64 interval = u64(«agg.interval» * 1e9) * «rollup_count»;
			«ELSE»
				constexpr u64 interval = u64(«agg.interval» * 1e9);
			«ENDIF»

			auto &store = «store_name»;
			auto &queue = store.current_queue();

			s16 relative_timeslot = store.relative_timeslot(t);

			if (relative_timeslot <= 0) {
				/* not ready */
				return;
			}

			double slot_duration = store.slot_duration();
			u64 metric_timestamp = t - (u64)(relative_timeslot * slot_duration);

			while (!queue.empty()) {
				/* get the next loc */
				u32 loc = queue.peek();
				/* get the metrics entry for that loc */
				«IF agg.isRoot && !is_rollup»
					auto &metrics = store.lookup_relative(loc, 0, false).second.get_metrics();
				«ELSE»
					auto &metrics = store.lookup_relative(loc, 0, false).second;
				«ENDIF»
				/* get a reference to the span with the metric */
				auto span = at(loc);

				/* call the functor */
				f(metric_timestamp, span, metrics, interval);

				«IF !is_rollup»
				/* propagate updates on the aggregation tree */
				«FOR update : agg.updates»
					/* update «update.ref.name».«update.agg.name»: */
					{
						auto update_weak_ref = span.«update.ref.name»();
						if (update_weak_ref.valid()) {
							update_weak_ref.«update.agg.name»_update(metric_timestamp, metrics);
						}
					}
				«ENDFOR»

				«FOR rollup : agg.rollups»
					{
						/* time-based rollup to «rollup.rollup_count» times of interval */
						auto loc = span.loc();
						«generateMetricUpdate(agg, agg.name + "_" + rollup.rollup_count, "loc", "t", "metrics", false)»
					}
				«ENDFOR»
				«ENDIF»

				/* return the reference count for the metric */
				put(loc);

				queue.pop();
			}

			/* done. advance the current timeslot */
			store.advance();
		}
		'''
	}


	/***************************************************************************
	 * Proxy methods
	 **************************************************************************/

	static def proxyMethodDeclaration(Message msg) {
		'''
		void «msg.name»(«msg.norefPrototype»);
		void «msg.name»_tstamp(u64 ts«msg.norefCommaPrototype»);
		'''
	}

	static def proxyMethodDefinition(Span span, Message msg) {
		'''

		void «span.name»::«msg.name»(«msg.norefPrototype»)
		{
			«proxyMethodInvocation(span, msg, 'index_', 'loc_', 'span_ptr_->shard_id_')»
		}

		void «span.name»::«msg.name»_tstamp(u64 ts«msg.norefCommaPrototype»)
		{
			«proxyMethodTimestampInvocation(span, msg, 'index_', 'ts', 'loc_', 'span_ptr_->shard_id_')»
		}
		'''
	}

	static def proxyMethodInvocation(Span span, Message msg, String index, String ref, String shard_id) {
		val remote_app_name = msg.span.app.name;
		val writers = index + '.' + remote_app_name + '_writers_';

		if (span.sharding !== null) {
			'''
			auto &writer = «writers»[«shard_id»];
			writer.«msg.name»(«ref»«msg.norefCommaCallPrototype»);
			'''
		} else {
			'''
			for (auto &writer : «writers») {
				writer.«msg.name»(«ref»«msg.norefCommaCallPrototype»);
			}
			'''
		}
	}

	static def proxyMethodInvocation(Span span, Message msg, String index, String ref) {
		proxyMethodInvocation(span, msg, index, ref, 'shard_id');
	}

	static def proxyMethodTimestampInvocation(Span span, Message msg, String index, String ts, String ref, String shard_id) {
		val remote_app_name = msg.span.app.name;
		val writers = index + '.' + remote_app_name + '_writers_';

		if (span.sharding !== null) {
			'''
			auto &writer = «writers»[«shard_id»];
			writer.«msg.name»_tstamp(«ts», «ref»«msg.norefCommaCallPrototype»);
			'''
		} else {
			'''
			for (auto &writer : «writers») {
				writer.«msg.name»_tstamp(«ts», «ref»«msg.norefCommaCallPrototype»);
			}
			'''
		}
	}
	/***************************************************************************
	 * FIELD HELPER FUNCTIONS
	 **************************************************************************/

	static def integerTypeSize(FieldTypeEnum enum_type) {
		switch (enum_type) {
				case FieldTypeEnum.S8,
				case FieldTypeEnum.U8: 1
				case FieldTypeEnum.S16,
				case FieldTypeEnum.U16: 2
				case FieldTypeEnum.S32,
				case FieldTypeEnum.U32: 4
				case FieldTypeEnum.S64,
				case FieldTypeEnum.U64: 8
				case FieldTypeEnum.S128,
				case FieldTypeEnum.U128: 16
				case FieldTypeEnum.STRING:
					throw new RuntimeException("String not supported in hash")
		}
	}

	static def fieldSize(Field field) {
		val non_array_size =
			if (field.type.isShortString)
				field.type.size
			else
				integerTypeSize(field.type.enum_type)

		if (field.isArray)
			non_array_size * field.array_size
		else
			non_array_size
	}

	static def generateField(Field field) {
		generateField(field, "")
	}

	static def generateField(Field field, String variable_prefix) {
		'''«field.cType» «variable_prefix»«field.name»;'''
	}

	static def generateFieldTypeInfo(Field field) {
		'''typedef «field.cType» «field.name»_t;'''
	}

	/***************************************************************************
	 * HANDLE HELPER FUNCTIONS
	 **************************************************************************/
	static def locationTypeForHandle(Span span) {
		switch span.pool_size {
			case span.pool_size < (1 << 16): 	"u16"
			default:	"u32"
		}
	}

	static def invalidConstForHandle(Span span) {
		switch span.pool_size {
			case span.pool_size < (1 << 16): 	((1L << 16) - 1)
			default:							((1L << 32) - 1)
		}
	}

	/***************************************************************************
	 * INDEX H
	 **************************************************************************/

	static def indexConstructorSignature(App app) {
		'''«FOR ran : app.remoteApps.map[name].sort SEPARATOR ", "»std::vector<::«app.pkg.name»::«ran»::Writer> «ran»_writers«ENDFOR»'''
	}

	static def generateIndexH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "containers.h"
		«FOR remote_app : app.remoteApps»
		#include "../«remote_app.name»/writer.h"
		«ENDFOR»

		#include <functional>
		#include <ostream>
		#include <string>
		#include <vector>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		/**
		 * Index: a container for all types of spans, with reference counting.
		 *
		 * Each container for a type of span keeps constant-sized pools of that
		 * span, maps so spans can be found by keys, and metric stores for
		 * metrics associated with the span
		 */
		class Index {
		public:
			/**
			 * C'tor
			 */
			Index(«indexConstructorSignature(app)»);

			«FOR span : app.spans»
				/**
				 * Container for type «span.name»
				 */
				containers::«span.name» «span.name»;
			«ENDFOR»

			/**
			 * Extract size statistics for each container type.
			 *
			 * The given functor is called with (span name, num_allocated_spans, pool_size)
			 */
			using size_statistics_cb =
				std::function<void(
					std::string_view span_name,
					std::size_t allocated,
					std::size_t max_allocated,
					std::size_t pool_size)>;
			void size_statistics(size_statistics_cb f);

			/**
			 * forbid copy constructor
			 */
			Index(const Index&) = delete;

			/**
			 * forbid assignment operator
			 */
			Index& operator=(const Index&) = delete;

			/**
			 * Send a heartbeat pulse to all downstream peers
			 */
			void send_pulse();

			«FOR remote_app_name : app.remoteApps.map[name].sort»
				/* Writer for sending proxy span messages to «remote_app_name» app */
				std::vector<::«pkg_name»::«remote_app_name»::Writer> «remote_app_name»_writers_;
			«ENDFOR»

			void dump_json(std::ostream &out) const;

			friend std::ostream &operator <<(std::ostream &out, Index const &what) {
				what.dump_json(out);
				return out;
			}
		};
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * INDEX CC
	 **************************************************************************/

	static def generateIndexCc(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»

		#include "index.h"

		«pkg_name»::«app.name»::Index::Index(«indexConstructorSignature(app)»)
		«FOR span : app.spans BEFORE " : " SEPARATOR ","»
			«span.name»()
		«ENDFOR»
		«FOR ran : app.remoteApps.map[name].sort BEFORE ", " SEPARATOR ","»
			«ran»_writers_(std::move(«ran»_writers))
		«ENDFOR»
		{}

		void «pkg_name»::«app.name»::Index::size_statistics(size_statistics_cb f)
		{
			«FOR span : app.spans»
				f("«span.name»", «span.name».size(), «span.name».max_size(), «span.pool_size»);
			«ENDFOR»
		}

		void «pkg_name»::«app.name»::Index::send_pulse()
		{
			«FOR ran : app.remoteApps.map[name].sort»
				for (auto &writer : «ran»_writers_) {
					writer.«pulseMessageName»();
				}
			«ENDFOR»
		}

		void «pkg_name»::«app.name»::Index::dump_json(std::ostream &out) const
		{
			out << '{'
				«FOR span: app.spans SEPARATOR " << ','"»
					<< "\"«span.name»\": " << «span.name»
				«ENDFOR»
				<< '}';
		}
		'''
	}


	/***************************************************************************
	 * CONTAINERS H
	 **************************************************************************/

	static def allocParamList(Span span) {
		if (span.sharding !== null) {
			'''«FOR field : span.sharding.keys SEPARATOR ", "»«field.cType» «field.name»«ENDFOR»'''
		} else {
			""
		}
	}

	static def generateContainersH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "spans.h"
		#include "keys.h"
		#include "auto_handles.h"

		#include <generated/«pkg_name»/metrics.h>

		#include <util/short_string.h>
		#include <util/fixed_hash.h>
		#include <util/metric_store.h>

		#include <ostream>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace containers {

		«FOR span : app.spans»
			/**
			 * Container for span «span.name».
			 *
			 * The container maintains a constant-sized pool from which spans are
			 * allocated, and if the span is indexd, a map from the index key
			 * to the spans.
			 *
			 * Access to elements is performed through handles which keep a reference
			 * to an allocated span, or through a weak reference ("weak_ref"),
			 * which allows read and write access to span fields.
			 */
			class «span.name» {
			public:
				/**
				 * pool_size: size of the constant-sized pool of spans available
				 */
				static constexpr u32 pool_size = «span.pool_size»;

				/**
				 * C'tor
				 */
				«span.name»();

				«IF span.index !== null»
				/**
				 * Get a span handle from key.
				 *
				 * This span is indexed, so spans are accessed by their key.
				 * @see ::«pkg_name»::«app.name»::keys::«span.name»
				 */
				auto_handles::«span.name» by_key(const ::«pkg_name»::«app.name»::keys::«span.name» &key, bool create_if_not_found = true);
				«ELSE»
				/**
				 * Allocate a new span.
				 *
				 * This span is unindexed. Use alloc() to get a new span that can
				 * then be modified using weak_ref::«span.name»::modify().
				 */
				auto_handles::«span.name» alloc(«allocParamList(span)»);
				«ENDIF»

				/**
				 * get: increase reference count and get a handle for an existing span
				 */
				::«pkg_name»::«app.name»::auto_handles::«span.name» get(«locationTypeForHandle(span)» loc);

				/**
				 * put: decrease reference count and deallocate span if new refcount is 0.
				 *
				 * @return true if this was the last reference, false otherwise.
				 */
				bool put(«locationTypeForHandle(span)» loc);

				/**
				 * Get a specific span
				 *
				 * @assumes: refcount for the span is positive.
				 */
				::«pkg_name»::«app.name»::weak_refs::«span.name» at(«locationTypeForHandle(span)» loc);

				/**
				 * @return number of allocated spans of type «span.name»
				 */
				std::size_t size() const;

				/**
				 * @return maximum number of allocated spans of type «span.name»
				 */
				std::size_t max_size() const;

				/***********************
				 * Metrics
				 */
				«FOR agg : span.aggs»
				/**
				 * aggregator «agg.name»: update metrics
				 */
					«IF agg.isRoot»
					void «agg.name»_update(«locationTypeForHandle(span)» loc, u64 t, ::«pkg_name»::metrics::«agg.type.name»_point const &m);
					«ELSE»
					void «agg.name»_update(«locationTypeForHandle(span)» loc, u64 t, ::«pkg_name»::metrics::«agg.type.name» const &m);
					«ENDIF»
				«ENDFOR»

				«FOR agg : span.aggs»
					/**
					 * aggregator «agg.name»: is timeslot ready for output
					 *
					 * @returns true if the aggregation has a timeslot ready
					 */
					bool «agg.name»_ready(u64 t);
					«FOR rollup : agg.rollups»

					/**
					 * «agg.name»_«rollup.rollup_count»: is timeslot ready for output.
					 */
					bool «agg.name»_«rollup.rollup_count»_ready(u64 t);
					«ENDFOR»
				«ENDFOR»

				«FOR agg : span.aggs»
					/**
					 * aggregator «agg.name»: process timeslot with functor
					 *
					 * For each live entry in timeslot, calls functor.operator()
					 * with (t_of_timeslot, loc_in_index, span, metric),
					 * then advances the timeslot
					 */
					template<class FUNCTOR>
					void «agg.name»_foreach(u64 t, FUNCTOR &&f);

					«FOR rollup : agg.rollups»
					/**
					 * aggregator rollup «agg.name»_«rollup.rollup_count»: process timeslot with functor
					 *
					 * For each live entry in timeslot, calls functor.operator()
					 * with (t_of_timeslot, loc_in_index, span, metric),
					 * then advances the timeslot
					 */
					template<class FUNCTOR>
					void «agg.name»_«rollup.rollup_count»_foreach(u64 t, FUNCTOR &&f);
					«ENDFOR»
				«ENDFOR»

				using span_t = ::«pkg_name»::«app.name»::spans::«span.name»;
				«IF span.index !== null»
				typedef ::«pkg_name»::«app.name»::keys::«span.name» key_t;
				«ENDIF»

				«IF span.sharding !== null»
					struct sharding_key_t {
						«FOR field : span.sharding.keys»
							«field.cType» «field.name»;
						«ENDFOR»
					};

					static inline std::size_t hash_sharding_key(sharding_key_t const &key);
				«ENDIF»

				/* getter for specific location */
				friend ::«pkg_name»::«app.name»::weak_refs::«span.name»;
				inline ::«pkg_name»::«app.name»::spans::«span.name» *at_ptr(«locationTypeForHandle(span)» loc);

				«IF span.index !== null»
					/* key hasher for map */
					struct hasher_t {
						typedef std::size_t result_type;
						inline result_type operator()(key_t const &key) const noexcept;
					};

					/* key equality for map */
					struct equals_t {
						inline bool operator()(key_t const &lhs, key_t const &rhs) const noexcept;
					};

					/* map type */
					using map_t = FixedHash<key_t, span_t, pool_size, hasher_t, equals_t>;

					map_t map;
				«ELSE»
					/* pool */
					Pool<span_t, pool_size> map;
				«ENDIF»

				/* metric stores */
				«FOR agg: span.aggs»
				«IF agg.isRoot»
					MetricStore<::«pkg_name»::metrics::«agg.type.name»_accumulator, pool_size, «agg.slots»> «agg.name»;
				«ELSE»
					MetricStore<::«pkg_name»::metrics::«agg.type.name», pool_size, «agg.slots»> «agg.name»;
				«ENDIF»
				«FOR rollup: agg.rollups»
					MetricStore<::«pkg_name»::metrics::«agg.type.name», pool_size, «agg.slots»> «agg.name»_«rollup.rollup_count»;
				«ENDFOR»
				«ENDFOR»

				void dump_json(std::ostream &out) const;

				friend std::ostream &operator <<(std::ostream &out, «span.name» const &what) {
					what.dump_json(out);
					return out;
				}
			};

		«ENDFOR»
		} /* namespace containers */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * CONTAINERS INL
	 **************************************************************************/
	static def generateContainersInl(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "containers.h"
		#include "weak_refs.h"
		#include "weak_refs.inl"
		#include "modifiers.h"
		#include <util/container_of.h>
		#include <util/lookup3.h>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace containers {
			/*****************************************************************************
			 * hashers
			 ****************************************************************************/
			«FOR span : app.spans.filter[index !== null]»
				inline «span.name»::hasher_t::result_type
				«span.name»::hasher_t::operator()(«span.name»::key_t const &key) const noexcept
				{
					«generateKeyHashingFuncImpl(span.index.keys.filter(Field), span.index.keys.filter(Reference))»
				}
			«ENDFOR»

			«FOR span : app.spans.filter[sharding !== null]»
				inline std::size_t «span.name»::hash_sharding_key(«span.name»::sharding_key_t const &key)
				{
					«generateKeyHashingFuncImpl(span.sharding.keys, #[])»
				}
			«ENDFOR»

			/*****************************************************************************
			 * key equality
			 ****************************************************************************/
			«FOR span : app.spans.filter[index !== null]»

			bool «span.name»::equals_t::operator()(«span.name»::key_t const &lhs, «span.name»::key_t const &rhs) const noexcept
			{
				return
					/* references and plain fields are XORed together */
					((0
					«FOR defn : span.index.keys.filter(Reference)»
					| (lhs.«defn.name» ^ rhs.«defn.name»)
					«ENDFOR»
					«FOR defn : span.index.keys.filter(Field).filter[!type.isShortString && !isArray]»
					| (lhs.«defn.name» ^ rhs.«defn.name»)
					«ENDFOR»
					) == 0)
					/* strings and arrays have to be compared */
					«FOR field : span.index.keys.filter(Field).filter[type.isShortString || isArray]»
						&& (lhs.«field.name» == rhs.«field.name»)
					«ENDFOR»
					;
			}
			«ENDFOR»

		/*****************************************************************************
		 * span getters
		 ****************************************************************************/
		«FOR span : app.spans»
		::«pkg_name»::«app.name»::spans::«span.name» *«span.name»::at_ptr(«locationTypeForHandle(span)» loc)
		{
			return &map[loc];
		}
		«ENDFOR»

		/*****************************************************************************
		 * metrics iterators
		 ****************************************************************************/
		«FOR span : app.spans»
			«FOR agg : span.aggs»
				«generateMetricForeach(agg, span.name, false, 1)»

				«FOR rollup : agg.rollups»
					«generateMetricForeach(agg, span.name, true, rollup.rollup_count)»
				«ENDFOR»
			«ENDFOR»
		«ENDFOR»
		} /* namespace containers */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
	'''
	}

	static def generateKeyHashingFuncImpl(Field[] fields, Reference[] references) {
		'''
		u32 val = 0x7AFBAF00;

		/**** fields ****/
		«FOR field : fields»
			«IF field.isArray && field.type.isShortString»
				/* «field.name» is an array of strings. hash each one. */
				for (int i = 0; i < field.array_size; i++) {
					val = lookup3_hashlittle(key.«field.name»[i].buf, key.«field.name»[i].len, val + key.«field.name»[i].len);
				}
			«ELSEIF field.type.isShortString»
				/* «field.name» is a string */
				val = lookup3_hashlittle(key.«field.name».buf, key.«field.name».len, val + key.«field.name».len);
			«ELSEIF integerTypeSize(field.type.enum_type) % 4 == 0»
				/* «field.name» is a primitive type, is multiple of 4 bytes. will hash in 4-byte words */
				val = lookup3_hashword((u32 *)&key.«field.name», «fieldSize(field) / 4», val + «fieldSize(field)»);
			«ELSE»
				/* «field.name» is a plain variable: will hash individual bytes */
				val = lookup3_hashlittle((char *)&key.«field.name», «fieldSize(field)», val + «fieldSize(field)»);
			«ENDIF»
		«ENDFOR»

		«IF references.size > 0»
			/**** references ****/
			val = lookup3_hashword(key.references, «references.size», val + (4 * «references.size»));
		«ENDIF»

		return val;
		'''
	}

	/***************************************************************************
	 * CONTAINERS CC
	 **************************************************************************/

	static def generateContainersCc(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»

		#include "containers.h"
		#include "index.h"
		#include <util/container_of.h>
		#include <util/fast_div.h>
		#include "containers.inl"

		«FOR remote_app : app.remoteApps»
		#include "../«remote_app.name»/writer.h"
		«ENDFOR»

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace containers {

		«FOR span : app.spans»
			/*****************************************************************************
			 * «span.name»
			 ****************************************************************************/
			/* c'tor */
			«span.name»::«span.name»()
			«FOR agg: span.aggs BEFORE " : " SEPARATOR ","»
				«agg.name»(fast_div(double(«agg.interval» * 1e9), 16))
				«FOR rollup: agg.rollups BEFORE " , " SEPARATOR ","»
					«agg.name»_«rollup.rollup_count»(fast_div(double(«agg.interval» * 1e9) * «rollup.rollup_count», 16))
				«ENDFOR»
			«ENDFOR»
			{}

			«IF span.index !== null»
			::«pkg_name»::«app.name»::auto_handles::«span.name»
			«span.name»::by_key(const ::«pkg_name»::«app.name»::keys::«span.name» &key, bool create_if_not_found)
			{
				/* does the key already have a handle? */
				auto pos = map.find(key);
				if(pos.index == map_t::invalid) {
					auto index_ptr = fp_container_of(this, &Index::«span.name»);

					if (!create_if_not_found) {
						return {*index_ptr};
					}

					/* could not find, insert */
					pos = map.insert(key);
					if (pos.index == map_t::invalid) {
						/* could not insert into the map, it's probably full */
						return {*index_ptr};
					}

					/* make the handle */
					::«pkg_name»::«app.name»::auto_handles::«span.name» handle(*index_ptr, pos.index);

					«IF span.sharding !== null»
						/* calculate the shard ID of this span */
						size_t num_remote_instances = index_ptr->«span.remoteApp.name»_writers_.size();
						assert(num_remote_instances <= std::numeric_limits<decltype(handle.span_ptr_->shard_id_)>::max());
						auto shard_id = hash_sharding_key({«FOR field : span.sharding.keys SEPARATOR ", "»key.«field.name»«ENDFOR»}) % num_remote_instances;
						handle.span_ptr_->shard_id_ = shard_id;
					«ENDIF»

					/* set the key values in the entry */
					handle.modify()
						«FOR defn : span.index.keys»
							«IF defn instanceof Reference»
								.«defn.name»(index_ptr->«defn.target.name».get(key.«defn.name»))
							«ELSE»
								.«defn.name»(key.«defn.name»)
							«ENDIF»
						«ENDFOR»
						;

					«IF span.isProxy»
					{
						/* call remote span start message */
						«IF span.remoteSpan.index !== null»
							/* the remote span is indexed; send key fields in the start message */
							«FOR key_field : span.remoteSpan.index.keys»
								«IF (key_field instanceof Field) && (key_field as Field).isArray»
									auto «key_field.name» = key.«key_field.name».data();
								«ELSE»
									auto &«key_field.name» = key.«key_field.name»;
								«ENDIF»
							«ENDFOR»
						«ENDIF»
						«proxyMethodInvocation(span, span.proxyStartMessage, '(*index_ptr)', 'pos.index')»
					}
					«ENDIF»

					/* have a valid value_type with refcount = 1 */
					return handle;
				} else {
					/* found an existing handle. get a refcount */
					return get(pos.index);
				}
			}

			«ELSE»
			/* new span allocator */
			auto_handles::«span.name» «span.name»::alloc(«allocParamList(span)»)
			{
				auto index_ptr = fp_container_of(this, &Index::«span.name»);
				auto loc = map.emplace().index;

				if (loc == map.invalid) {
					return {*index_ptr, loc};
				}

				auto_handles::«span.name» handle(*index_ptr, loc);

				«IF span.sharding !== null»
					/* calculate the shard ID of this span */
					size_t num_remote_instances = index_ptr->«span.remoteApp.name»_writers_.size();
					assert(num_remote_instances <= std::numeric_limits<decltype(handle.span_ptr_->shard_id_)>::max());
					auto shard_id = hash_sharding_key({«FOR field : span.sharding.keys SEPARATOR ", "»«field.name»«ENDFOR»}) % num_remote_instances;
					handle.span_ptr_->shard_id_ = shard_id;

					/* set the values in the sharding key */
					handle.modify()
						«FOR field : span.sharding.keys»
							.«field.name»(«field.name»)
						«ENDFOR»
						;
				«ENDIF»

				«IF span.isProxy»
					/* call remote span start message */
					«proxyMethodInvocation(span, span.proxyStartMessage, '(*index_ptr)', 'loc')»
				«ENDIF»

				return handle;
			}

			«ENDIF»
			::«pkg_name»::«app.name»::auto_handles::«span.name» «span.name»::get(«locationTypeForHandle(span)» loc)
			{
				auto index_ptr = fp_container_of(this, &Index::«span.name»);

				if (loc == «invalidConstForHandle(span)») {
					return {*index_ptr};
				}

				map[loc].__refcount++;

				return {*index_ptr, loc};
			}

			/**
			 * Puts reference.
			 * @return true if this was the last reference, false otherwise.
			 */
			bool «span.name»::put(«locationTypeForHandle(span)» loc)
			{
				«IF (span.definitions.filter(Reference).size > 0) || span.isProxy»
				auto index_ptr = fp_container_of(this, &Index::«span.name»);
				«ENDIF»
				auto &val = map[loc];
				u32 new_count = --val.__refcount;
				if (new_count == 0) {
					«IF span.index !== null»
						key_t _key;
						«FOR field : span.index.keys.filter(Field)»
							_key.«field.name» = val.__«field.name»;
						«ENDFOR»
						«FOR defn : span.index.keys.filter(Reference)»
							_key.«defn.name» = val.__«defn.name»;
						«ENDFOR»
					«ENDIF»
					«IF span.definitions.filter(Reference).size > 0»
						/* put references to the referenced indices */
						«FOR defn : span.definitions.filter(Reference)»
							if (val.__«defn.name» != «invalidConstForHandle(defn.target)») {
								index_ptr->«defn.target.name».put(val.__«defn.name»);
								val.__«defn.name» = «invalidConstForHandle(defn.target)»;
							}
						«ENDFOR»
					«ENDIF»
					«IF span.sharding !== null»
						auto shard_id = val.shard_id_;
					«ENDIF»
					«IF span.index !== null»
						map.erase(_key);
					«ELSE»
						map.remove(loc);
					«ENDIF»
					«IF span.isProxy»
						/* call remote span end message */
						«proxyMethodInvocation(span, span.proxyEndMessage, '(*index_ptr)', 'loc')»
					«ENDIF»
					return true;
				}
				return false;
			}

			::«pkg_name»::«app.name»::weak_refs::«span.name» «span.name»::at(«locationTypeForHandle(span)» loc)
			{
				auto index_ptr = fp_container_of(this, &Index::«span.name»);
				return {*index_ptr, loc};
			}

			std::size_t «span.name»::size() const {
				return map.size();
			}

			std::size_t «span.name»::max_size() const {
				return map.max_size();
			}

			/* metric aggregators */
			«FOR agg : span.aggs»
			«IF agg.isRoot»
				void «span.name»::«agg.name»_update(«locationTypeForHandle(span)» loc,
						u64 t, ::«pkg_name»::metrics::«agg.type.name»_point const &m)
				{
					«generateMetricUpdate(agg, agg.name, "loc", "t", "m", true)»
				}
			«ELSE»
				void «span.name»::«agg.name»_update(«locationTypeForHandle(span)» loc,
						u64 t, ::«pkg_name»::metrics::«agg.type.name» const &m)
				{
					«generateMetricUpdate(agg, agg.name, "loc", "t", "m", false)»
				}
			«ENDIF»
			«ENDFOR»

			/* metrics, is timeslot ready */
			«FOR agg : span.aggs»
				bool «span.name»::«agg.name»_ready(u64 t)
				{
					auto relative = «agg.name».relative_timeslot(t);
					return (relative > 0);
				}
				«FOR rollup : agg.rollups»
					bool «span.name»::«agg.name»_«rollup.rollup_count»_ready(u64 t)
					{
						auto relative = «agg.name»_«rollup.rollup_count».relative_timeslot(t);
						return (relative > 0);
					}
				«ENDFOR»
			«ENDFOR»

			void «span.name»::dump_json(std::ostream &out) const
			{
				out << "{\"spans\":[";

				bool first = true;
				for (auto const i: map.allocated()) {
					if (first) {
						first = false;
					} else {
						out << ',';
					}
					out << "{\"@ref\":" << i << ',' << map[i] << '}';
				}

				out << "]}";
			}
		«ENDFOR»
		} /* namespace containers */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * KEYS H
	 **************************************************************************/
	static def generateKeysH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>
		#include <util/short_string.h>
		#include <array>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace keys {
		«FOR span : app.spans»
			«IF span.index !== null»
				/**
				 * Key struct for span «span.name»
				 *
				 * This struct is used as a key to the mapping that indexes the span
				 */
				struct «span.name» {
					«span.name»() = default;
					«IF span.index.keys.filter(Field).length > 0»
					«span.name»(«FOR field : span.index.keys.filter(Field) SEPARATOR ", "»«field.cType» «field.name»«ENDFOR»«FOR ref : span.index.keys.filter(Reference) BEFORE (span.index.keys.filter(Field).empty ? "" : ", ") SEPARATOR ", "»u32 «ref.name»«ENDFOR»):
						«FOR field : span.index.keys.filter(Field) SEPARATOR ","»
							«field.name»(«field.name»)
						«ENDFOR»
						«FOR ref : span.index.keys.filter(Reference) BEFORE (span.index.keys.filter(Field).empty ? "" : ",") SEPARATOR ","»
							«ref.name»(«ref.name»)
						«ENDFOR»
					{}
					«ENDIF»

					«span.name»(«span.name» const &) = default;
					«span.name»(«span.name» &&) = default;
					«span.name» &operator =(«span.name» const &) = default;
					«span.name» &operator =(«span.name» &&) = default;

					«FOR field : span.index.keys.filter(Field)»
					/* field «field.name» */
					using «field.name»_t = «field.cType»;
					«generateField(field)»
					«ENDFOR»

					/**
					 * References to other spans are kept in u32.
					 *
					 * This allows for fast hashing.
					 */
					union {
						u32 references[«span.index.keys.filter(Reference).size»];
						struct {
							«FOR ref : span.index.keys.filter(Reference)»
								u32 «ref.name»;
							«ENDFOR»
						};
					};
				};
			«ENDIF»
		«ENDFOR»
		} /* namespace keys */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * HANDLES H
	 **************************************************************************/
	static def generateHandlesH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		/* forward declarations */
		class Index;
		namespace containers {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace weak_refs {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace auto_handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace auto_handle_converters {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}

		namespace handles {
		«FOR span : app.spans»
			class «span.name» {
			public:
				using location_type = «locationTypeForHandle(span)»;
				static constexpr location_type invalid = «invalidConstForHandle(span)»;

				/**
				 * C'tor for an invalid handle.
				 *
				 * @note: a different constructor allows constructing a valid handle.
				 * that c'tor is used by containers to get references to spane.
				 * The c'tor is private to protect the consistency of reference counting.
				 */
				«span.name»();

				/**
				 * D'tor.
				 *
				 * Has an assert that the reference has been put or released.
				 */
				~«span.name»();

				/**
				 * Get the offset into the span pool of this handle
				 */
				location_type loc();

				/**
				 * @returns true if this handle is valid
				 */
				bool valid();

				/**
				 * Puts the reference back
				 *
				 * @param ai: the Index that holds the span corresponding to the reference
				 */
				bool put(Index &ai);

				/**
				 * Gets the offset and moves the reference out of the handle.
				 *
				 * @imporant: it is the responsibility of the caller to put() the reference
				 */
				location_type release();

				/**
				 * Get a weak_ref to access the span associated with the handle
				 */
				::«pkg_name»::«app.name»::weak_refs::«span.name» access(Index &ai);

				/**
				 * Move operator is allowed
				 */
				«span.name»& operator=(«span.name» &&other);

				/**
				 * Move constructor is allowed
				 */
				«span.name»(«span.name» &&other);

				/**
				 * Non-move copy constructor is forbidden
				 */
				«span.name»(const «span.name»&) = delete;

				/**
				 * Non-move assignment is forbidden
				 */
				«span.name»& operator=(const «span.name»&) = delete;

				/**
				 * Convert from an auto_handle_converter to a handle
				 */
				«span.name»(::«pkg_name»::«app.name»::auto_handle_converters::«span.name» &&other);

			private:
				friend class ::«pkg_name»::«app.name»::containers::«span.name»;
				friend class ::«pkg_name»::«app.name»::weak_refs::«span.name»;
				friend class ::«pkg_name»::«app.name»::auto_handles::«span.name»;

				explicit «span.name»(location_type loc);
				location_type loc_;
			};

		«ENDFOR»
		} /* namespace handles */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * HANDLES CC
	 **************************************************************************/
	static def generateHandlesCc(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»

		#include "handles.h"
		#include "index.h"
		#include <assert.h>
		#include "containers.h"
		#include "containers.inl"
		#include "weak_refs.h"
		#include "auto_handle_converters.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace handles {

		«FOR span : app.spans»
		/********************************************
		 * «span.name»
		 ********************************************/
		«span.name»::«span.name»()
			: loc_(invalid)
		{}

		«span.name»::~«span.name»()
		{
			assert(!valid());
		}

		«span.name»::location_type «span.name»::loc()
		{
			return loc_;
		}

		bool «span.name»::valid()
		{
			return loc_ != invalid;
		}

		bool «span.name»::put(Index &ai) {
			if (valid()) {
				bool res = ai.«span.name».put(loc_);
				loc_ = invalid;
				return res;
			}
			return false;
		}

		«span.name»::location_type «span.name»::release()
		{
			auto ret = loc_;
			loc_ = invalid;
			return ret;
		}

		/* access the span associated with the handle */
		::«pkg_name»::«app.name»::weak_refs::«span.name» «span.name»::access(Index &ai)
		{
			return ai.«span.name».at(loc_);
		}

		«span.name»& «span.name»::operator=(«span.name» &&other) {
			assert(!valid());
			loc_ = other.loc_;
			other.loc_ = invalid;
			return *this;
		}

		«span.name»::«span.name»(«span.name» &&other)
		{
			loc_ = other.loc_;
			other.loc_ = invalid;
		}

		«span.name»::«span.name»(«span.name»::location_type loc)
			: loc_(loc)
		{}

		«span.name»::«span.name»(::«pkg_name»::«app.name»::auto_handle_converters::«span.name» &&other)
		{
			loc_ = other.release();
		}

		«ENDFOR»
		} /* namespace handles */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * AUTO HANDLES H
	 **************************************************************************/
	static def generateAutoHandlesH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "weak_refs.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		/* forward declarations */
		class Index;
		namespace containers {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}

		namespace auto_handles {
		«FOR span : app.spans»
			/**
			 * Auto-handle for span «span.name».
			 *
			 * Like a weak_ref, but puts the reference after use
			 */
			class «span.name» : public ::«pkg_name»::«app.name»::weak_refs::«span.name» {
			public:
				/**
				 * C'tor for an invalid handle.
				 *
				 * @note: a different constructor allows constructing a valid handle.
				 * that c'tor is used by containers to get references to spane.
				 * The c'tor is private to protect the consistency of reference counting.
				 */
				«span.name»(Index &index);

				/**
				 * D'tor.
				 *
				 * Frees the reference to the object
				 */
				~«span.name»();

				/**
				 * Put the reference
				 */
				void put();

				/**
				 * Convert to handle for storage
				 *
				 * The reference moves to the handle, and this instance becomes
				 * invalid
				 */
				::«pkg_name»::«app.name»::handles::«span.name» to_handle();

				/**
				 * Release the reference, return the loc
				 */
				«locationTypeForHandle(span)» release();

				/**
				 * Move operator is allowed
				 */
				«span.name»& operator=(«span.name» &&other);

				/**
				 * ..but weak_ref is incompatible (does not hold a reference)
				 */
				«span.name»& operator=(::«pkg_name»::«app.name»::weak_refs::«span.name» &&other) = delete;

				/**
				 * Move constructor is allowed
				 */
				«span.name»(«span.name» &&other);

				/**
				 * ..but weak_ref is incompatible (does not hold a reference)
				 */
				«span.name»(::«pkg_name»::«app.name»::weak_refs::«span.name» &&other) = delete;

				/**
				 * Non-move copy constructor is forbidden
				 */
				«span.name»(const «span.name»&) = delete;

				/**
				 * Non-move assignment is forbidden
				 */
				«span.name»& operator=(const «span.name»&) = delete;

			private:
				friend class ::«pkg_name»::«app.name»::containers::«span.name»;

				/**
				 * C'tor from a location into the index.
				 *
				 * @assumes caller has taken a reference. This reference becomes the
				 * responsibility of this instance
				 */
				«span.name»(Index &index, location_type loc);
			};

		«ENDFOR»
		} /* namespace auto_handles */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * AUTO HANDLES CC
	 **************************************************************************/
	static def generateAutoHandlesCc(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»

		#include "auto_handles.h"
		#include "index.h"
		#include <assert.h>
		#include "containers.h"
		#include "containers.inl"
		#include "weak_refs.h"
		#include "handles.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace auto_handles {

		«FOR span : app.spans»
		/********************************************
		 * «span.name»
		 ********************************************/
		«span.name»::«span.name»(Index &index)
			: ::«pkg_name»::«app.name»::weak_refs::«span.name»(index, invalid)
		{}

		«span.name»::~«span.name»()
		{
			put();
		}

		void «span.name»::put()
		{
			if (valid()) {
				index_.«span.name».put(loc_);
				loc_ = invalid;
				span_ptr_ = nullptr;
			}
		}

		::«pkg_name»::«app.name»::handles::«span.name» «span.name»::to_handle()
		{
			«locationTypeForHandle(span)» saved_loc = loc_;
			loc_ = invalid;
			span_ptr_ = nullptr;
			return ::«pkg_name»::«app.name»::handles::«span.name»(saved_loc);
		}

		«locationTypeForHandle(span)» «span.name»::release()
		{
			«locationTypeForHandle(span)» saved_loc = loc_;
			loc_ = invalid;
			span_ptr_ = nullptr;
			return saved_loc;
		}

		«span.name»& «span.name»::operator=(«span.name» &&other)
		{
			put();

			loc_ = other.loc_;
			span_ptr_ = other.span_ptr_;
			other.loc_ = invalid;
			other.span_ptr_ = nullptr;

			return *this;
		}

		«span.name»::«span.name»(«span.name» &&other)
			: ::«pkg_name»::«app.name»::weak_refs::«span.name»(other.index_, other.loc_)
		{
			other.loc_ = invalid;
			other.span_ptr_ = nullptr;
		}

		«span.name»::«span.name»(Index &index, location_type loc)
			: ::«pkg_name»::«app.name»::weak_refs::«span.name»(index, loc)
		{}

		«ENDFOR»
		} /* namespace auto_handles */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * WEAK REFS H
	 **************************************************************************/
	static def String generateImplFwdDeclaration(String[] namespaces) {
		'''
		«IF namespaces.length === 1»
			class «namespaces.get(0)»;
		«ELSE»
			namespace «namespaces.get(0)» {
				«generateImplFwdDeclaration(Arrays.copyOfRange(namespaces, 1, namespaces.length))»
			} /* «namespaces.get(0)» */
		«ENDIF»
		'''
	}

	static def generateWeakRefsH(Resource resource, App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>
		#include <util/short_string.h>
		#include <jitbuf/jb.h>

		#include <array>

		/* Span implementation classes */
		«FOR span : app.spans.filter[impl !== null]»
			«generateImplFwdDeclaration(span.impl.split("::"))»
		«ENDFOR»

		namespace «pkg_name» { /* pkg */

		/* forward declarations */
		namespace metrics {
		«FOR metric : resource.allContents.filter(Metric).toIterable»
				class «metric.name»;
				class «metric.name»_point;
		«ENDFOR»
		}

		namespace «app.name» { /* app */

		/* forward declarations */
		class Index;
		namespace handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace auto_handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace modifiers {
		«FOR span : app.spans»
			class «span.name»;
		«ENDFOR»
		}
		namespace spans {
		«FOR span : app.spans»
			class «span.name»;
		«ENDFOR»
		}

		namespace impl {
		/* internal utility class: accessor used in the implementation */
		class __accessor;
		}

		namespace weak_refs {
		/* forward declarations */
		«FOR span : app.spans»
			class «span.name»;
		«ENDFOR»

		«FOR span : app.spans»
			/**
			 * Weak reference to the span.
			 *
			 * @assumes the referenced span is either invalid or has a handle that
			 * holds a reference.
			 */
			class «span.name» {
			public:
				using location_type = «locationTypeForHandle(span)»;
				static constexpr location_type invalid = «invalidConstForHandle(span)»;

				/**
				 * C'tor.
				 * @note: it might be preferable to obtain a weak_ref through
				 * the index, from another weak_ref, or via a handle.
				 *
				 * @param index: the index that allocated the span
				 * @param loc: the offset of the span in the span's pool
				 */
				inline «span.name»(Index &index, location_type loc);

				/**
				 * @returns true if reference is valid
				 */
				inline bool valid() const
				{
					return loc_ != invalid;
				}

				/**
				 * Get the offset into the span pool of this handle
				 */
				location_type loc() const
				{
					return loc_;
				}

				/**
				 * Get the reference count of the underlying span
				 */
				u32 refcount() const;

				/**
				 * Get the index object this span belongs to
				 */
				Index &index()
				{
					return index_;
				}

				/**
				 * get: increase reference count and get a handle to the span
				 */
				::«pkg_name»::«app.name»::auto_handles::«span.name» get();

				«FOR field : span.definitions.filter(Field) SEPARATOR "\n"»
				/**
				 * Getter for field «field.name»
				 *
				 * @assumes valid() == true
				 */
				using «field.name»_t = «field.cType»;
				«field.name»_t &«field.name»();
				«ENDFOR»

				«FOR ref : span.definitions.filter(Reference)»
				/**
				 * Getter for reference «ref.name».
				 *
				 * @returns: a weak_ref to the referenced span
				 *
				 * @assumes valid() == true
				 *
				 * @note using fully qualified return value in this definition is
				 * necessary because reference names can be equal to the targets
				 */
				::«pkg_name»::«app.name»::weak_refs::«ref.target.name» «ref.name»();
				«ENDFOR»

				/**
				 * Obtains a modifier for the span.
				 *
				 * The modifier allows changing fields and references, and automatically
				 * updates auto references when destructed.
				 */
				::«pkg_name»::«app.name»::modifiers::«span.name» modify();

				«FOR agg : span.aggs»
				/**
				 * Update aggregator «agg.name» of metric «agg.type.name» associated
				 * with this span.
				 *
				 * @param t: time of the telemetry
				 * @param m: the values of the metric to be aggreagated
				 */
				«IF agg.isRoot»
					void «agg.name»_update(u64 t, ::«pkg_name»::metrics::«agg.type.name»_point const &m);
				«ELSE»
					void «agg.name»_update(u64 t, ::«pkg_name»::metrics::«agg.type.name» const &m);
				«ENDIF»
				«ENDFOR»

				«IF span.isProxy»
					/* Proxy methods */
					«FOR msg : span.proxyLogMessages»
						«proxyMethodDeclaration(msg)»
					«ENDFOR»
				«ENDIF»

				«IF span.impl !== null»
					/* Accessor for the span's message handler instance */
					::«span.impl» &impl();
				«ENDIF»

				void dump_json(std::ostream &out) const;

				friend std::ostream &operator <<(std::ostream &out, «span.name» const &what) {
					what.dump_json(out);
					return out;
				}

			protected:
				/* allow spans to follow weak_refs using span_ptr_ */
				friend class impl::__accessor;

				Index &index_;
				location_type loc_;
				::«pkg_name»::«app.name»::spans::«span.name» *span_ptr_;
			};
		«ENDFOR»
		} /* namespace weak_refs */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * WEAK REFS INL
	 **************************************************************************/
	static def generateWeakRefsInl(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "weak_refs.h"
		#include "index.h"
		#include <util/container_of.h>
		#include <util/lookup3.h>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace weak_refs {
		«FOR span : app.spans»
			inline «span.name»::«span.name»(Index &index, «span.name»::location_type loc)
				: index_(index)
				, loc_(loc)
				, span_ptr_( (loc == invalid) ? nullptr : index.«span.name».at_ptr(loc_))
			{}

		«ENDFOR»
		} /* namespace weak_refs */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * WEAK REFS CC
	 **************************************************************************/
	static def generateWeakRefsCc(App app, String pkg_name) {
		'''
		#include "weak_refs.h"
		#include "spans.h"
		#include "keys.h"
		#include "index.h"
		#include "modifiers.h"
		#include "auto_handles.h"
		#include "weak_refs.inl"
		#include "containers.inl"

		«FOR remote_app : app.remoteApps»
		#include "../«remote_app.name»/writer.h"
		«ENDFOR»

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace weak_refs {
		«FOR span : app.spans»
			/*******************************
			 * «span.name»
			 *******************************/
			/**
			 * Get a reference to the node
			 */
			::«pkg_name»::«app.name»::auto_handles::«span.name» «span.name»::get()
			{
				assert(valid());
				return index_.«span.name».get(loc_);
			}

			/* field getters */
			«FOR field : span.definitions.filter(Field)»
				«field.cType» &«span.name»::«field.name»()
				{
					return span_ptr_->__«field.name»;
				}
			«ENDFOR»

			/* reference getters */
			«FOR ref : span.definitions.filter(Reference)»
				::«pkg_name»::«app.name»::weak_refs::«ref.target.name»
				«span.name»::«ref.name»()
				{
					«IF ref.isIsCached»
						/* cached reference: refresh cache */
						span_ptr_->refresh__«ref.name»(index_);
					«ENDIF»
					/* return weak_ref to referenced span */
					return ::«pkg_name»::«app.name»::weak_refs::«ref.target.name»(index_, span_ptr_->__«ref.name»);
				}
			«ENDFOR»

			u32 «span.name»::refcount() const
			{
				assert(valid());
				return span_ptr_->__refcount;
			}

			::«pkg_name»::«app.name»::modifiers::«span.name» «span.name»::modify()
			{
				return {span_ptr_, index_};
			}

			/* metric updaters */
			«FOR agg : span.aggs»
			«IF agg.isRoot»
			void «span.name»::«agg.name»_update(u64 t, ::«pkg_name»::metrics::«agg.type.name»_point const &m)
			«ELSE»
			void «span.name»::«agg.name»_update(u64 t, ::«pkg_name»::metrics::«agg.type.name» const &m)
			«ENDIF»
			{
				return index_.«span.name».«agg.name»_update(loc_, t, m);
			}
			«ENDFOR»

			«IF span.isProxy»
				/* Proxy methods */
				«FOR msg : span.proxyLogMessages»
					«proxyMethodDefinition(span, msg)»
				«ENDFOR»
			«ENDIF»

			«IF span.impl !== null»
				/* Get a ref to the span's accessor */
				::«span.impl» &«span.name»::impl() { return span_ptr_->impl_; };
			«ENDIF»

			void «span.name»::dump_json(std::ostream &out) const
			{
				if (valid()) {
					assert(span_ptr_);
					out << *span_ptr_;
				} else {
					out << "null";
				}
			}
		«ENDFOR»
		} /* namespace weak_refs */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}


	/***************************************************************************
	 * MODIFIERS H
	 **************************************************************************/
	static def generateModifiersH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>
		#include <util/short_string.h>
		#include <array>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		/* forward declarations */
		class Index;
		namespace handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace auto_handles {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace containers {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace weak_refs {
		«FOR span : app.spans»
			class «span.name»;
		«ENDFOR»
		}
		namespace spans {
		«FOR span : app.spans»
			class «span.name»;
		«ENDFOR»
		}

		namespace modifiers {
		«FOR span : app.spans»
			/**
			 * Modifier for span «span.name»
			 *
			 * Allows a user to change fields of the span.
			 *
			 * @note: if the span is indexed, fields that are part of the key are
			 * only settable from the container, specifically the by_key() method,
			 * so the fields are always consistent with the mapping. These fields
			 * are later used to remove the span from the index when the refcount
			 * for the span reaches 0.
			 */
			class «span.name» {
			public:
				/* note: c'tor is private (only available through the weak_ref's modify()) */

				/**
				 * D'tor
				 *
				 * The d'tor is responsible for recomputing auto references once all
				 * other updates have been applied
				 */
				~«span.name»();

				/**
				 * Non-move copy constructor is forbidden
				 */
				«span.name»(const «span.name»&) = delete;

				/**
				 * Non-move assignment is forbidden
				 */
				«span.name»& operator=(const «span.name»&) = delete;

				/***************************************************
				 * Field types
				 ***************************************************/
				«FOR field : span.definitions.filter(Field)»
					«generateFieldTypeInfo(field)»
				«ENDFOR»

				/***************************************************
				 * Field setters
				 ***************************************************/
				«FOR field : span.definitions.filter(Field)»
					«IF !((span.index !== null) && (span.index.keys.contains(field)))»
						/**
						 * Setter for field «field.name»
						 */
						«span.name» &«field.name»(const «field.cType» &_«field.name»);
					«ENDIF»
				«ENDFOR»
				/***************************************************
				 * Manual references
				 ***************************************************/
				«FOR ref : span.definitions.filter(Reference).filter[!isAuto && !isCached]»
					«IF !((span.index !== null) && (span.index.keys.contains(ref)))»
						/**
						 * Setter for reference «ref.name» to span «ref.target.name»
						 */
						«span.name» &«ref.name»(::«pkg_name»::«app.name»::handles::«ref.target.name» &&other);
						/**
						 * Setter for reference «ref.name» to span «ref.target.name»
						 */
						«span.name» &«ref.name»(::«pkg_name»::«app.name»::auto_handles::«ref.target.name» &&other);
					«ENDIF»
				«ENDFOR»
			private:
				/***************************************************
				 * fields in index (read-only by users)
				 ***************************************************/
				«FOR field : span.definitions.filter(Field)»
					«IF (span.index !== null) && (span.index.keys.contains(field))»
						/**
						 * Setter for field «field.name»
						 *
						 * This field is indexed on, so the setter is private
						 */
						«span.name» &«field.name»(const «field.cType» &_«field.name»);
					«ENDIF»
				«ENDFOR»
				/***************************************************
				 * manual references in index (read-only by users)
				 ***************************************************/
				«FOR ref : span.definitions.filter(Reference).filter[!isAuto && !isCached]»
					«IF (span.index !== null) && (span.index.keys.contains(ref))»
						/**
						 * Setter for reference «ref.name» to span «ref.target.name»
						 *
						 * This reference is indexed on, so the setter is private
						 */
						«span.name» &«ref.name»(::«pkg_name»::«app.name»::handles::«ref.target.name» &&other);
						/**
						 * Setter for reference «ref.name» to span «ref.target.name»
						 *
						 * This reference is indexed on, so the setter is private
						 */
						«span.name» &«ref.name»(::«pkg_name»::«app.name»::auto_handles::«ref.target.name» &&other);
					«ENDIF»
				«ENDFOR»

				/* allow the by_key() method to set the keys */
				friend class ::«pkg_name»::«app.name»::containers::«span.name»;

				/* allow the weak_ref to construct instances */
				friend class ::«pkg_name»::«app.name»::weak_refs::«span.name»;

				/**
				 * Private c'tor
				 *
				 * @param span_ptr: pointer to the span we're modifying
				 * @param index: the index holding the span. used to get and put
				 * references.
				 */
				«span.name»(::«pkg_name»::«app.name»::spans::«span.name» *span_ptr, Index &index);

				::«pkg_name»::«app.name»::spans::«span.name» *span_ptr_;
				Index &index_;
				u64 modified_mask_;
			};
		«ENDFOR»
		} /* namespace modifiers */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * Modifiers CC
	 **************************************************************************/
	static def generateModifiersCc(App app, String pkg_name) {
		'''
		#include "modifiers.h"
		#include "spans.h"
		#include "keys.h"
		#include "index.h"
		#include "containers.h"
		#include "handles.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */
		namespace modifiers {
		«FOR span : app.spans»
			«generateModifierImpl(app, span, pkg_name)»
		«ENDFOR»
		} /* namespace modifiers */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}
	static def generateModifierImpl(App app, Span span, String pkg_name) {
		val deps = new SpanAutoDependencies(span)

		if (deps.non_dynamic_prereqs.size > 64)
			throw new RuntimeException(
				 '''span «span.name» has «deps.non_dynamic_prereqs.size» prereqs, only 64 supported''')

		'''
		/*******************************
		 * «span.name» modifier masks
		 *******************************/
		namespace {
			struct «span.name»__masks {
				/* enum of all possible prereqs */
				enum class prereqs { «deps.non_dynamic_prereqs.map[name].join(', ')» };
				/* mask of each prereq */
				«FOR defn : deps.non_dynamic_prereqs»
					static constexpr u64 «defn.name» = (1ull << static_cast<int>(prereqs::«defn.name»));
				«ENDFOR»
				/* mask for each computed reference, what are its dependencies */
				«FOR ref : deps.compute_order»
					static constexpr u64 «ref.name» = «deps.ref_non_dynamic_prereqs.get(ref).map[name].join(' | ')»;
				«ENDFOR»
			};
		}

		/*******************************
		 * «span.name»
		 *******************************/
		«span.name»::«span.name»(::«pkg_name»::«app.name»::spans::«span.name» *span_ptr, Index &index)
			: span_ptr_(span_ptr),
				index_(index),
				modified_mask_(0)
		{
			assert(span_ptr_ != nullptr);
		}

		«span.name»::~«span.name»()
		{
			/* compute order: «deps.compute_order.map[name].join(", ")» */
			«FOR ref: deps.compute_order»
				if (modified_mask_ & «span.name»__masks::«ref.name») {
					span_ptr_->refresh__«ref.name»(index_);
				}
			«ENDFOR»
		}

		/* fields */
		«FOR field : span.definitions.filter(Field)»
			«span.name» &«span.name»::«field.name»(const «field.cType» &_«field.name»)
			{
				span_ptr_->__«field.name» = _«field.name»;
				«IF deps.non_dynamic_prereqs.contains(field)»
					modified_mask_ |= «span.name»__masks::«field.name»;
				«ENDIF»
				return *this;
			}
		«ENDFOR»
		/* manual references */
		«FOR ref : span.definitions.filter(Reference).filter[!isAuto && !isCached]»
			«FOR handle_type: Arrays.asList("handles", "auto_handles")»
			«span.name» &«span.name»::«ref.name»(::«pkg_name»::«app.name»::«handle_type»::«ref.target.name» &&other)
			{
				if (span_ptr_->__«ref.name» != «invalidConstForHandle(ref.target)») {
					index_.«ref.target.name».put(span_ptr_->__«ref.name»);
				}
				span_ptr_->__«ref.name» = other.release();
				«IF deps.non_dynamic_prereqs.contains(ref)»
					modified_mask_ |= «span.name»__masks::«ref.name»;
				«ENDIF»
				return *this;
			}
			«ENDFOR»
		«ENDFOR»

		'''
	}

	/***************************************************************************
	 * SPANS H
	 **************************************************************************/
	static def generateSpansH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>
		#include <util/short_string.h>
		#include <array>

		/* Span implementation classes */
		«FOR app_span : app.spans.filter[include !== null]»
			#include «app_span.include»
		«ENDFOR»

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		/* forward declarations */
		class Index;
		namespace keys {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace weak_refs {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace modifiers {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}
		namespace containers {
			«FOR span : app.spans»
				class «span.name»;
			«ENDFOR»
		}

		namespace impl {
		/* internal utility class: accessor used in the implementation */
		class __accessor;
		}

		namespace spans {
		«FOR span : app.spans»
		class «span.name» {
		public:
			/**
			 * C'tor
			 */
			«span.name»();

			/**
			 * Field types.
			 */
			«FOR field : span.definitions.filter(Field)»
				«generateFieldTypeInfo(field)»
			«ENDFOR»

			void dump_json(std::ostream &out) const;

			friend std::ostream &operator <<(std::ostream &out, «span.name» const &what) {
				what.dump_json(out);
				return out;
			}

		private:
			/* allow getters from the weak_ref */
			friend class ::«pkg_name»::«app.name»::weak_refs::«span.name»;

			/* allow modification from modifier */
			friend class ::«pkg_name»::«app.name»::modifiers::«span.name»;

			/* allow access to the accessor for compute_key__<...> methods */
			friend class impl::__accessor;

			/* allow refcounting */
			friend class ::«pkg_name»::«app.name»::containers::«span.name»;

			/**
			 * compute key for auto and cached references
			 *
			 * @returns: true if key is valid, false otherwise
			 */
			«FOR ref : span.definitions.filter(Reference).filter[isAuto || isCached]»
				bool compute_key__«ref.name»(Index &index, ::«pkg_name»::«app.name»::keys::«ref.target.name» &key);
			«ENDFOR»

			/**
			 * refresh auto and cached references, after recomputing key
			 */
			«FOR ref : span.definitions.filter(Reference).filter[isAuto || isCached]»
				void refresh__«ref.name»(Index &index);
			«ENDFOR»

			/* reference count */
			u32 __refcount;

			«IF span.sharding !== null»
				/* remote span's shard */
				u8 shard_id_;
			«ENDIF»

			/* fields */
			«FOR field : span.definitions.filter(Field)»
				«generateField(field, "__")»
			«ENDFOR»

			/* references */
			«FOR ref : span.definitions.filter(Reference)»
				«locationTypeForHandle(ref.target)» __«ref.name»;
			«ENDFOR»

			«IF span.impl !== null»
			/* span impl */
			::«span.impl» impl_;
			«ENDIF»
		};

		«ENDFOR»
		} /* namespace spans */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}


	/***************************************************************************
	 * SPANS CC
	 **************************************************************************/
	static def generateSpansCc(App app, String pkg_name) {
		'''
		#include "spans.h"
		#include "weak_refs.h"
		#include "keys.h"
		#include "index.h"
		#include "containers.inl"

		#include <util/raw_json.h>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		namespace impl {
		class __accessor {
			public:
			«FOR span : app.spans»
				«FOR field : span.definitions.filter(Field)»
					inline static «field.cType»& «span.name»__«field.name»(weak_refs::«span.name» ref) { return ref.span_ptr_->__«field.name»; }
				«ENDFOR»
				«FOR ref : span.definitions.filter(Reference)»
					inline static «locationTypeForHandle(ref.target)» «span.name»__«ref.name»(weak_refs::«span.name» ref) { return ref.span_ptr_->__«ref.name»; };
				«ENDFOR»
			«ENDFOR»
		};
		} /* namespace impl */

		namespace spans {
		«FOR span : app.spans»
		/********************************************
		 * «span.name»
		 ********************************************/
		«span.name»::«span.name»()
			: __refcount(1)
			«IF span.sharding !== null»
			, shard_id_(0)
			«ENDIF»
			«FOR ref: span.definitions.filter(Reference) BEFORE ", " SEPARATOR ", "»
				__«ref.name»(«invalidConstForHandle(ref.target)»)
			«ENDFOR»
		{}

		/* key getters for auto, cached references */
		«FOR ref : span.definitions.filter(Reference).filter[isAuto || isCached]»
			bool «span.name»::compute_key__«ref.name»(Index &index, ::«pkg_name»::«app.name»::keys::«ref.target.name» &key)
			{
				«FOR binding : ref.bindings»
				/* find key '«binding.key.name»' */
				{
					«generateReferenceRef(binding.value, "key." + binding.key.name, 0)»
				}
				«ENDFOR»
				return true;
			}
		«ENDFOR»

		/* refresh references */
		«FOR ref : span.definitions.filter(Reference).filter[isAuto || isCached]»
			void «span.name»::refresh__«ref.name»(Index &index)
			{
				/* auto reference '«ref.name»' */
				auto prev_reference = __«ref.name»;

				/* compute the key from dependencies */
				::«pkg_name»::«app.name»::keys::«ref.target.name» key;
				bool valid = compute_key__«ref.name»(index, key);

				/* get the reference, if valid */
				if (valid) {
					__«ref.name» = index.«ref.target.name».by_key(key).release();
				} else {
					__«ref.name» = «invalidConstForHandle(ref.target)»;
				}

				/* put the previous reference if it was valid */
				if (prev_reference != «invalidConstForHandle(ref.target)»)
					index.«ref.target.name».put(prev_reference);
			}
		«ENDFOR»

		void «span.name»::dump_json(std::ostream &out) const
		{
			out << "\"@refcount\":" << __refcount;
			«FOR field : span.definitions.filter(Field)»
				print_json_value(out << ",\"«field.name»\":", __«field.name»);
			«ENDFOR»
			«FOR ref : span.definitions.filter(Reference)»
				out << ",\"#«ref.name»\":" << __«ref.name»;
			«ENDFOR»
		}
		«ENDFOR»
		} /* namespace spans */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/**
	 * Resolve a reference binding for compute_key__<...>
	 */
	static def CharSequence generateReferenceRef(ReferenceBindingRef ref, String into_var, Integer depth)
	{
		switch ref {
			ReferenceBindingRoot:
				'''«into_var» = __«ref.entity.name»;'''
			ReferenceBindingValue: {
				val span = ref.tail.eContainer as Span
				val ref_handle = '''__ref_handle_«depth»'''
				'''
				««« first, prepare a variable to save the handle into
				«locationTypeForHandle(span)» «ref_handle»;
				««« generate code recursively into ref_handle_«depth»
				«generateReferenceRef(ref.ref, ref_handle, depth+1)»
				««« if handle is invalid, return false
				if («ref_handle» == «invalidConstForHandle(span)») { return false; }
				««« resolve the reference and get the definition
				«into_var» = impl::__accessor::«span.name»__«ref.tail.name»(index.«span.name».at(«ref_handle»));
				'''
			}
		}
	}

	/***************************************************************************
	 * AUTO HANDLE CONVERTER H
	 **************************************************************************/
	static def generateAutoHandleConvertersH(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include "auto_handles.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		namespace auto_handle_converters {
		«FOR span : app.spans»
			class «span.name» : public ::«pkg_name»::«app.name»::auto_handles::«span.name»{
			public:
				/**
				 * C'tor
				 */
				«span.name»(::«pkg_name»::«app.name»::auto_handles::«span.name» &&«span.name»);

				/**
				 * Move c'tor
				 */
				«span.name»(«span.name» &&other);
			};

		«ENDFOR»
		} /* namespace auto_handle_converters */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * AUTO HANDLE CONVERTER CC
	 **************************************************************************/
	static def generateAutoHandleConvertersCc(App app, String pkg_name) {
		'''
		«generatedCodeWarning()»

		#include <utility>
		#include "auto_handle_converters.h"

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		namespace auto_handle_converters {
		«FOR span : app.spans»
			/********************************************
			* «span.name»
			********************************************/
			«span.name»::«span.name»(::«pkg_name»::«app.name»::auto_handles::«span.name» &&«span.name»)
				: ::«pkg_name»::«app.name»::auto_handles::«span.name»(std::move(«span.name»))
			{}

		«ENDFOR»
		} /* namespace auto_handle_converters */
		} /* namespace «app.name» (app) */
		} /* namespace «pkg_name» (pkg) */
		'''
	}

	/***************************************************************************
	 * META H
	 **************************************************************************/

	static def generateMetaH(App app, String pkg_name) {
		return '''
		«generatedCodeWarning()»
		#pragma once

		#include <generated/«pkg_name»/«app.name».parsed_message.h>
		#include <generated/«pkg_name»/«app.name».wire_message.h>
		#include <generated/«pkg_name»/«app.name»/protocol.h>
		#include <generated/«pkg_name»/«app.name»/transform_builder.h>

		#include <util/meta.h>

		#include <string_view>
		#include <type_traits>

		#include <cstdint>

		namespace «pkg_name» { /* pkg */
		namespace «app.name» { /* app */

		«FOR span : app.spans»
			«FOR msg : span.messages»
				struct «msg.name»_message_metadata {
					static constexpr std::uint16_t rpc_id = «msg.wire_msg.rpc_id»;
					static constexpr std::string_view name = "«msg.name»";

					using wire_message = «msg.wire_msg.struct_name»;
					static constexpr std::size_t wire_message_size = «msg.wire_msg.size»;

					using parsed_message = «msg.parsed_msg.struct_name»;
					static constexpr std::size_t parsed_message_size = «msg.parsed_msg.size»;

					«FOR field : msg.fields.indexed»
						struct field_«field.value.name» {
							using type = «msg.parsed_msg.cType(field.value.type)»«field.value.arraySuffix»;
							static constexpr std::string_view name = "«field.value.name»";
							static constexpr std::size_t index = «field.key»;
							static constexpr auto const &get(void const *msg) {
								return reinterpret_cast<parsed_message const *>(msg)->«field.value.name»;
							}
						};

					«ENDFOR»
					using fields = meta::list<«FOR field : msg.fields SEPARATOR ", "»field_«field.name»«ENDFOR»>;

					«IF msg.reference_field !== null»
						static constexpr bool has_reference = true;
						using reference = field_«msg.reference_field.name»;
					«ELSE»
						static constexpr bool has_reference = false;
					«ENDIF»
				};

			«ENDFOR»
		«ENDFOR»
		} // namespace «app.name» /* app */

		class «app.name»_metadata {

			«FOR span : app.spans»
				«FOR msg : span.messages»
					static «app.name»::«msg.name»_message_metadata message_metadata_for_impl(
							«msg.wire_msg.struct_name» const &);
					static «app.name»::«msg.name»_message_metadata message_metadata_for_impl(
							«msg.parsed_msg.struct_name» const &);
				«ENDFOR»
			«ENDFOR»

		public:
			using protocol = «app.name»::Protocol;
			using transform_builder = «app.name»::TransformBuilder;

			using messages = meta::list<«FOR msg : app.spans.flatMap[messages] SEPARATOR ", "»«app.name»::«msg.name»_message_metadata«ENDFOR»>;

			template <typename MessageStruct>
			using message_metadata_for = decltype(
				message_metadata_for_impl(std::declval<MessageStruct>())
			);
		};

		} // namespace «pkg_name» /* pkg */
		'''
	}
}
