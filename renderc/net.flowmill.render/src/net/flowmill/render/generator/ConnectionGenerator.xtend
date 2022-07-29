// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import net.flowmill.render.render.App
import static extension net.flowmill.render.extensions.AppExtensions.*
import static extension net.flowmill.render.extensions.SpanExtensions.*
import static extension net.flowmill.render.extensions.MessageExtensions.*
import static extension net.flowmill.render.extensions.FieldTypeExtensions.*
import net.flowmill.render.render.Field
import net.flowmill.render.render.Span
import net.flowmill.render.render.Message
import net.flowmill.render.render.MessageType

class ConnectionGenerator {
	static def generateConnectionH(App app)
	{
		'''
		/********************************************
		 * JITBUF GENERATED CODE
		 * !!! generated code, do not modify !!!
		 ********************************************/

		#pragma once

		#include <stdexcept>
		#include <common/client_type.h>
		#include <platform/types.h>
		#include <util/fixed_hash.h>
		#include <util/lookup3.h>
		#include "index.h"
		#include "handles.h"

		/* Span implementation classes */
		«FOR app_span : app.spans.filter[include !== null]»
			#include «app_span.include»
		«ENDFOR»

		namespace «app.pkg.name» {
		namespace «app.name» {

		/* forward declaration */
		class Protocol;

		/******************************************************************************
		 * CONNECTION CLASS: handles a single connection to the server
		 ******************************************************************************/
		class Connection {
		public:
			/**
			 * C'tor
			 * @param identity: True if all messages should be identity messages
			 */
			Connection(Protocol &protocol, Index &index);

			/**
			 * D'tor
			 */
			~Connection();

			/**
			 * Auth support.
			 */
			void on_connection_authenticated();

			/**
			 * Singleton span accessors
			 */
			«FOR span : app.spans.filter[isSingleton]»
				weak_refs::«span.name» «span.name»() { return «span.instanceName».access(index_); }
			«ENDFOR»

			/**
			 * Handlers for all the incoming messages
			 */
			«FOR span : app.spans»
				/* span «span.name» */
				«FOR msg : span.messages»
				void «app.c_name»_«msg.name»(u64 timestamp, char *msg_buf);
				«ENDFOR»
			«ENDFOR»

			/* hashers for each span type */
			«FOR span : app.spans.filter[conn_hash]»
				struct «fixedHashHasherName(span)» {
					typedef std::size_t result_type;
					inline result_type operator()(«span.referenceType.wireCType» const &s) const noexcept;
				};
			«ENDFOR»

			/* hash table types for each span type */
			«FOR span : app.spans.filter[conn_hash]»
				using «fixedHashTypeName(span)» = FixedHash<«span.referenceType.wireCType», handles::«span.name»,
						  «span.pool_size», «fixedHashHasherName(span)»>;
			«ENDFOR»

			/* pools for each span type */
			«FOR span : app.spans.filter[conn_hash]»
			    «fixedHashTypeName(span)» «fixedHashName(span)»;
			«ENDFOR»

			/* singleton spans maintain one instance per span */
			«FOR span : app.spans.filter[isSingleton]»
				handles::«span.name» «span.instanceName»;
			«ENDFOR»

			/* lookup functions for each span type */
			«FOR span : app.spans.filter[conn_hash]»
				«spanLookupDeclaration(span)»
			«ENDFOR»

			/**
			 * Message statistics for «app.connName»
			 */
			struct Statistics {
				/**
				 * C'tor
				 */
				Statistics();

				/**
				 * For each message, calls @f (module_name, message_name, severity, count)
				 */
				void for_each_message(std::function<void(std::string_view, std::string_view, int, u64)> const& f);

				/*** message counts ***/
				struct {
					«FOR span : app.spans»
						/* span «span.name» */
						«FOR msg : span.messages»
							u64 «app.c_name»_«msg.name»;
						«ENDFOR»
					«ENDFOR»
				} counts;

				/*** message change indication ***/
				struct {
					«FOR span : app.spans»
						/* span «span.name» */
						«FOR msg : span.messages»
							unsigned int «app.c_name»_«msg.name»: 1; /* 1 bit */
						«ENDFOR»
					«ENDFOR»
				} changed;

			} statistics;

			struct MessageErrors {
				MessageErrors();

				/**
				 * For each message, calls @f (module_name, message_name, error_name, count)
				 */
				void foreach(std::function<void(std::string_view, std::string_view, std::string_view, u64)> const &f);

				struct {
					«FOR span : app.spans»
						«FOR msg : span.messages»
							«FOR error_name : msg.errors»
								u64 «app.c_name»_«msg.name»_«error_name»;
							«ENDFOR»
						«ENDFOR»
					«ENDFOR»
				} counts;

				struct {
					«FOR span : app.spans»
						«FOR msg : span.messages»
							«FOR error_name : msg.errors»
								unsigned int «app.c_name»_«msg.name»_«error_name» : 1;
							«ENDFOR»
						«ENDFOR»
					«ENDFOR»
				} changed;

			} message_errors;

			std::string_view get_client_location() const {
				return client_location_;
			}

			ClientType get_client_type() const {
				return client_type_;
			}

			void set_client_info(std::string_view host, ClientType type) {
				client_location_ = host;
				client_type_ = type;
			}

			Index &index() { return index_; }

		private:
			/* the protocol instance */
			Protocol &protocol_;
			Index &index_;
			std::string_view client_location_ = {};
			ClientType client_type_ = ClientType::unknown;
		};

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */
		'''
	}

	static def generateConnectionCc(App app)
	{
		'''
		#include "connection.h"
		#include "protocol.h"
		#include "auto_handle_converters.h"

		#include <util/log.h>
		#include <util/render.h>

		#include <algorithm>

		/* message structs for decoding */
		#include "generated/«app.jsrv_h»"

		namespace «app.pkg.name» {
		namespace «app.name» {

		/******************************************************************************
		 * Message statistics
		 ******************************************************************************/
		Connection::Statistics::Statistics()
		{
			«FOR span : app.spans»
				/* span «span.name» */
				«FOR msg : span.messages»
					counts.«app.c_name»_«msg.name» = 0;
				«ENDFOR»
			«ENDFOR»

			memset(&changed, 0, sizeof(changed));
		}

		void Connection::Statistics::for_each_message(std::function<void(std::string_view, std::string_view, int, u64)> const& f)
		{
			«FOR span : app.spans»
				/* span «span.name» */
				«FOR msg : span.messages»
					if (changed.«app.c_name»_«msg.name») {
						f("«app.c_name»", "«msg.name»", «msg.severity», counts.«app.c_name»_«msg.name»);
					}
				«ENDFOR»
			«ENDFOR»

			memset(&changed, 0, sizeof(changed));
		}

		/******************************************************************************
		 * Message error counts
		 ******************************************************************************/
		Connection::MessageErrors::MessageErrors()
		{
			memset(&counts, 0, sizeof(counts));
			memset(&changed, 0, sizeof(changed));
		}

		void Connection::MessageErrors::foreach(std::function<void(std::string_view, std::string_view, std::string_view, u64)> const &f)
		{
			«FOR span : app.spans»
				«FOR msg : span.messages»
					«FOR error_name : msg.errors»
						if (changed.«app.c_name»_«msg.name»_«error_name») {
							f("«app.c_name»", "«msg.name»", "«error_name»", counts.«app.c_name»_«msg.name»_«error_name»);
						}
					«ENDFOR»
				«ENDFOR»
			«ENDFOR»

			memset(&changed, 0, sizeof(changed));
		}

		/******************************************************************************
		 * CONNECTION CLASS: c'tor
		 ******************************************************************************/
		Connection::Connection(Protocol &protocol, Index &index)
			: protocol_(protocol),
				index_(index)
		{
			«FOR span : app.spans.filter[isSingleton]»
				{
					auto ref = index_.«span.name».alloc();
					if (!ref.valid())
						throw std::runtime_error("Connection::Connection - could not allocate «span.name» handle");
					«span.instanceName» = ref.to_handle();
				}
			«ENDFOR»

			/* populate handlers_ */
			«FOR span : app.spans»
				«FOR msg : span.messages»
					«IF msg.noAuthorizationNeeded»
						protocol_.add_handler(«msg.wire_msg.rpc_id», this,
							&dispatch_protocol_member_handler<Connection, &Connection::«app.c_name»_«msg.name»>);
					«ENDIF»
				«ENDFOR»
			«ENDFOR»

			/* we currently only support identity transformations */
			protocol_.insert_no_auth_identity_transforms();
		}

		/******************************************************************************
		 * CONNECTION CLASS: d'tor
		 ******************************************************************************/
		Connection::~Connection()
		{
			/* cleanup singleton spans */
			«FOR span : app.spans.filter[isSingleton]»
				«span.instanceName».put(index_);
			«ENDFOR»

			/* cleanup fixed hashes */
			«FOR span : app.spans.filter[conn_hash]»
				for (auto handle_loc : «fixedHashName(span)».allocated()) {
					«FOR msg : span.messages.filter[type == MessageType.END && reference_field.name == "_ref"]»
						{
							struct «msg.parsed_msg.struct_name» msg = {
								._rpc_id = «msg.wire_msg.rpc_id»,
								._ref = handle_loc,
							};
							auto span_ref = «fixedHashName(span)»[handle_loc].access(index_);
							span_ref.impl().«msg.name»(span_ref, 0, &msg);
						}
					«ENDFOR»
					«fixedHashName(span)»[handle_loc].put(index_);
				}
			«ENDFOR»
		}

		void Connection::on_connection_authenticated()
		{
			«FOR span : app.spans»
				«FOR msg : span.messages»
					«IF !msg.noAuthorizationNeeded»
						protocol_.add_handler(«msg.wire_msg.rpc_id», this,
							&dispatch_protocol_member_handler<Connection, &Connection::«app.c_name»_«msg.name»>);
					«ENDIF»
				«ENDFOR»
			«ENDFOR»

			/* we currently only support identity transformations */
			protocol_.insert_need_auth_identity_transforms();
		}

		/******************************************************************************
		 * CONNECTION CLASS: span accessor functions implementation
		 ******************************************************************************/
		«FOR span : app.spans.filter[conn_hash]»
			«spanLookupImplementation(span)»
		«ENDFOR»

		/******************************************************************************
		 * CONNECTION CLASS: handler function implementation
		 ******************************************************************************/
		«FOR span : app.spans»
			/* span «span.name» */
			«FOR msg : span.messages»
				«handlerImplementation(span, msg, app)»

			«ENDFOR»
		«ENDFOR»

		/******************************************************************************
		 * CONNECTION CLASS: hasher_t implementation for spans
		 ******************************************************************************/
		«FOR span : app.spans.filter[conn_hash]»
			typename Connection::«fixedHashHasherName(span)»::result_type
			Connection::«fixedHashHasherName(span)»::operator()(«span.referenceType.wireCType» const &s) const noexcept
			{
				/* note: this is from generateContainersInl */
				result_type val = 0xBFFB7A00;
				/**** fields ****/
				«FOR field : #[span.messages.head.reference_field]»
					«IF field.isArray && field.type.isShortString»
					«ELSEIF field.type.isShortString»
					«ELSEIF RenderGenerator::integerTypeSize(field.type.enum_type) % 4 == 0»
						/* «field.name» is a primitive type, is multiple of 4 bytes.
						   will hash in 4-byte words */
						val = (result_type)lookup3_hashword((u32 *)&s, «RenderGenerator::fieldSize(field) / 4», val + «RenderGenerator::fieldSize(field)»);
					«ELSE»
						/* «field.name» is a plain variable: will hash individual bytes */
						val = (result_type)lookup3_hashlittle((char *)&s, «RenderGenerator::fieldSize(field)», val + «RenderGenerator::fieldSize(field)»);
					«ENDIF»
				«ENDFOR»
				return val;
			}
		«ENDFOR»

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */
		'''
	}

	static def spanLookupDeclaration(Span span)
	{
		'''
		«fixedHashTypeName(span)»::position «fixedHashName(span)»_find(«span.referenceType.wireCType» key);
		'''
	}

	static def spanLookupImplementation(Span span)
	{
		'''
		Connection::«fixedHashTypeName(span)»::position Connection::«fixedHashName(span)»_find(«span.referenceType.wireCType» key)
		{
			return «fixedHashName(span)».find(key);
		}
		'''
	}

	static def handlerImplementation(Span span, Message msg, App app)
	{
		val pmsg = msg.parsed_msg

		'''
		void Connection::«app.c_name»_«msg.name»(u64 t, char *msg_buf)
		{
			struct «pmsg.struct_name» *msg = (struct «pmsg.struct_name» *)msg_buf;

			LOG::trace_in(
				client_type_, "[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
				" entered handler (rpc_id={})",
				client_type_, client_location_, msg->_rpc_id
			);

			«IF msg.type == MessageType.START»
				/* start message: insert */
				«IF span.index === null»
					auto ref = index_.«span.name».alloc();
				«ELSE»
					keys::«span.name» key;
					«FOR field : span.index.keys»
						«IF (field instanceof Field) && (field as Field).isIsArray»
							static_assert(sizeof(key.«field.name») == sizeof(msg->«field.name»));
							std::copy_n(std::begin(msg->«field.name»), «(field as Field).array_size», 
										std::begin(key.«field.name»));
						«ELSE»
							key.«field.name» = msg->«field.name»;
						«ENDIF»
					«ENDFOR»
					auto ref = index_.«span.name».by_key(key);
				«ENDIF»

				if (!ref.valid()) {
					LOG::trace_in(client_type_,
						"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
						" Could not allocate «span.name» ref",
						client_type_, client_location_);
					message_errors.counts.«app.c_name»_«msg.name»_span_alloc_failed += 1;
					message_errors.changed.«app.c_name»_«msg.name»_span_alloc_failed = 1;
					return;
				}
				auto converter = auto_handle_converters::«span.name»(std::move(ref));

				auto pos = «fixedHashName(span)».insert(msg->«msg.reference_field.name», std::move(converter));
				if (pos.index == «fixedHashName(span)».invalid) {
					if («fixedHashName(span)».full()) {
						LOG::trace_in(client_type_,
							"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
							" full (size={})",
							client_type_, client_location_, «fixedHashName(span)».full());
						message_errors.counts.«app.c_name»_«msg.name»_span_pool_full += 1;
						message_errors.changed.«app.c_name»_«msg.name»_span_pool_full = 1;
					} else {
						auto find_pos = «fixedHashName(span)».find(msg->«msg.reference_field.name»);
						if (find_pos.index != «fixedHashName(span)».invalid) {
							LOG::trace_in(client_type_,
								"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
								" already exists - {}",
								client_type_, client_location_, msg->«msg.reference_field.name»
							);
							message_errors.counts.«app.c_name»_«msg.name»_duplicate_ref += 1;
							message_errors.changed.«app.c_name»_«msg.name»_duplicate_ref = 1;
						} else {
							LOG::trace_in(client_type_,
								"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
								" insert failed (size={})",
								client_type_, client_location_, «fixedHashName(span)».full()
							);
							message_errors.counts.«app.c_name»_«msg.name»_span_insert_failed += 1;
							message_errors.changed.«app.c_name»_«msg.name»_span_insert_failed = 1;
						}
					}
					LOG::trace_in(client_type_,
						"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
						" ignoring message - can't allocate or lookup span",
						client_type_, client_location_
					);
					return;
				}
			«ELSEIF (!span.isSingleton)»
				/* get the destination span */
				auto pos = «fixedHashName(span)».find(msg->«msg.reference_field.name»);
				if (pos.index == «fixedHashName(span)».invalid) {
					LOG::trace_in(client_type_,
						"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
						" find failed key='{}' _ref={}",
						client_type_, client_location_,
						msg->«msg.reference_field.name», pos.index
					);
					message_errors.counts.«app.c_name»_«msg.name»_span_find_failed += 1;
					message_errors.changed.«app.c_name»_«msg.name»_span_find_failed = 1;
					return;
				}
			«ENDIF»

			«IF span.impl !== null»
			{
				/* call the span's handler */
				«IF span.isSingleton»
				auto entry = &«span.instanceName»;
				«ELSE»
				auto entry = pos.entry;
				«ENDIF»
				auto span_ref = entry->access(index_);
				LOG::trace_in(
					client_type_, "[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
					" delegating to «span.impl»",
					client_type_, client_location_
				);
				span_ref.impl().«msg.name»(span_ref, t, msg);
			}
			«ENDIF»

			«IF msg.type == MessageType.END»
				/* end message: remove from hash */
				«fixedHashName(span)»[pos.index].put(index_);
				bool erase_res = «fixedHashName(span)».erase(msg->«msg.reference_field.name»);
				if (erase_res != true) {
					LOG::trace_in(client_type_,
						"[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
						" erase failed",
						client_type_, client_location_
					);
					message_errors.counts.«app.c_name»_«msg.name»_span_erase_failed += 1;
					message_errors.changed.«app.c_name»_«msg.name»_span_erase_failed = 1;
				}
			«ENDIF»

			/* update message statistics */
			statistics.counts.«app.c_name»_«msg.name»++;
			statistics.changed.«app.c_name»_«msg.name» = 1;

			LOG::trace_in(
				client_type_, "[{} at '{}'] Connection::«app.c_name»_«msg.name»:"
				" left handler normally",
				client_type_, client_location_
			);
		}
		'''
	}
}
