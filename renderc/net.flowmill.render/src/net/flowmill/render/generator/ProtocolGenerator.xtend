//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

package net.flowmill.render.generator

import net.flowmill.render.render.App
import static extension net.flowmill.render.extensions.AppExtensions.*

class ProtocolGenerator {

	static def generateProtocolH(App app, String pkg_name)
	{
		val app_name = app.name
		'''
		/********************************************
		 * JITBUF GENERATED CODE
		 * !!! generated code, do not modify !!!
		 ********************************************/

		#pragma once

		#include <common/client_type.h>
		#include <jitbuf/perfect_hash.h>
		#include <platform/types.h>
		#include <generated/«pkg_name»/«app_name»/hash.h>
		«IF app.jit»
			#include <jitbuf/transform_builder.h>
		«ENDIF»

		#include <stdexcept>
		#include <chrono>

		namespace «app.pkg.name» {
		namespace «app.name» {

		/* forward declaration */
		class TransformBuilder;

		/******************************************************************************
		 * PROTOCOL CLASS: handles messages for a single connection
		 ******************************************************************************/
		class Protocol {
		public:
			/* message format transform function type */
			typedef uint16_t (*transform)(const char *src, char *dst);

			/* handler function type */
			typedef void (*handler_func_t)(void *context, u64 timestamp, char *msg_buf);

			/**
			 * C'tor
			 */
			Protocol(TransformBuilder &builder);

			struct handle_result_t {
				int result;
				std::chrono::nanoseconds client_timestamp;
			};

			/**
			 * handle a message
			 * @returns: the client's timestamp, as well as:
			 *   the message length on success or an error code,
			 *   -ENOENT if message was not added
			 *   -EACCES if message was not authenticated
			 *   -EAGAIN if buffer is too small
			 *   note that handler function might throw.
			 */
			handle_result_t handle(const char *msg, uint32_t len);

			/**
			 * Handles multiple consecutive messages
			 * @returns: the client's timestamp, as well as:
			 *    the length of successfully consumed messages if at least one
			 *    message was processed, otherwise like handle()
			 */
			handle_result_t handle_multiple(const char *msg, u64 len);

			/**
			 * Adds a handler function for the given RPC.
			 *
			 * @param rpc_id: the RPC's ID
			 * @param context: the context the handler is called on
			 * @param handler_fn: the handler
			 */
			void add_handler(u16 rpc_id, void *context, handler_func_t handler_fn);

			«IF app.jit»
				/**
				 * inserts the transform for the given RPC
				 */
				void insert_transform(u16 rpc_id, transform xform, u32 size,
						std::shared_ptr<jitbuf::TransformRecord> &transform_ptr);
			«ENDIF»

			/**
			 * insert an identity transform for the given RPC ID
			 */
			void insert_identity_transform(u16 rpc_id);

			/**
			 * inserts default identity transforms for no-auth messages
			 */
			void insert_no_auth_identity_transforms();

			/**
			 * inserts default identity transforms for need-auth messages
			 */
			void insert_need_auth_identity_transforms();

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

		private:
			/* hash function for PerfectHash objects */
			struct rpc_id_hash_fn {
				u32 operator()(u32 key) { return «app.hashName»(key); }
			};

			TransformBuilder &builder_;

			/* information about our implemented messages */
			struct func_info {
				void *context;
				handler_func_t handler_fn;
			};
			PerfectHash<func_info, «app.hashSize», rpc_id_hash_fn> funcs_;

			/* information about handlers and transforms for processing messages */
			struct handler_info {
				transform xform;
				void *context;
				handler_func_t handler_fn;
				u32 size;
				«IF app.jit»
					std::shared_ptr<jitbuf::TransformRecord> transform_ptr;
				«ENDIF»
			};
			PerfectHash<handler_info, «app.hashSize», rpc_id_hash_fn> handlers_;

			std::string_view client_location_ = {};
			ClientType client_type_ = ClientType::unknown;
		};

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */

		'''
	}

	static def generateProtocolCc(App app)
	{
		/* compute an upper bound on parsed message size */
		val max_message_size =
			if (app.spans.flatMap[messages].size == 0)
				0
			else
				app.spans.flatMap[messages.map[parsed_msg.size]].max

		val need_auth_msg = app.spans.flatMap[messages]
			.filter[!noAuthorizationNeeded];

		'''
		#include "protocol.h"
		#include "transform_builder.h"
		#include <iostream>
		#include <util/log.h>

		/* message structs for decoding */
		#include "generated/«app.jsrv_h»"
		#include "generated/«app.jb_h»"

		#include <spdlog/fmt/fmt.h>

		namespace «app.pkg.name» {
		namespace «app.name» {

		/******************************************************************************
		 * PROTOCOL CLASS: C'tor
		 ******************************************************************************/
		Protocol::Protocol(TransformBuilder &builder)
			: builder_(builder)
		{}


		/******************************************************************************
		 * PROTOCOL CLASS: incoming buffer handler
		 ******************************************************************************/
		Protocol::handle_result_t Protocol::handle(const char *msg, uint32_t len)
		{
			«IF app.spans.size == 0»
				/* no spans */
				LOG::error("«app.name»::Protocol::handle: no spans");
				return {.result = -EINVAL, .client_timestamp = std::chrono::nanoseconds::zero()};
			«ELSE»
				/* size check: should have enough for timestamp and rpc_id */
				if (len < sizeof(u64) + sizeof(u16)) {
					LOG::trace_in(
						client_type_, "[{} at '{}']: «app.name»::Protocol::handle:"
						" not enough data to read headers", client_type_, client_location_
					);
					return {.result = -EAGAIN, .client_timestamp = std::chrono::nanoseconds::zero()};
				}

				/* Handle timestamps */
				std::chrono::nanoseconds remote_timestamp{*(u64 const *)msg};

				msg += sizeof(u64);
				len -= sizeof(u64);

				/* get RPC ID */
				uint16_t rpc_id = *(uint16_t *)msg;

				/* find handler for RPC ID */
				handler_info *record = handlers_.find(rpc_id);
				LOG::trace_in(client_type_,
					"[{} at '{}']: «app.name»::Protocol::handle: resolved rpc_id and handler"
					" (available={} rpc={} handler={} context={})",
					client_type_, client_location_,
					len, rpc_id, reinterpret_cast<void const *>(record), reinterpret_cast<void const *>(record ? record->context : nullptr)
				);

				if (record == nullptr) {
					// compile-time list of rpc ids that need authentication
					constexpr std::size_t need_auth_rpc_ids_count = «need_auth_msg.length»;
					constexpr u16 need_auth_rpc_ids[] = {«FOR rpc_id : need_auth_msg.map[wire_msg].map[rpc_id].sort SEPARATOR ", "»«rpc_id»«ENDFOR»};

					if (std::binary_search(need_auth_rpc_ids, need_auth_rpc_ids + need_auth_rpc_ids_count, rpc_id)) {
						LOG::trace_in(client_type_, "handle(): permission denied for rpc_id: {}", rpc_id);
						return {.result = -EACCES, .client_timestamp = remote_timestamp};
					} else {
						LOG::trace_in(client_type_, "handle(): cannot find handler for rpc_id: {}", rpc_id);
						return {.result = -ENOENT, .client_timestamp = remote_timestamp};
					}
				}

				/* safety check message size */
				if (len < record->size) {
					LOG::trace_in(client_type_,
						"[{} at '{}']: «app.name»::Protocol::handle: not enough data"
						" (available={}) to read static payload (needed={} rpc={})",
						client_type_, client_location_,
						len, record->size, rpc_id
					);
					return {.result = -EAGAIN, .client_timestamp = remote_timestamp};
				}

				/* transform the message */
				u64 dst_buffer[(«max_message_size» + 7) / 8]; /* 64-bit aligned dst */
				uint16_t size = record->xform(msg, (char *)dst_buffer);

				/* if we didn't get all the dynamic sized part, request more bytes */
				if (size > len) {
					LOG::trace_in(client_type_,
						"[{} at '{}']: «app.name»::Protocol::handle: not enough data"
						" (available={}) to read dynamic payload (needed={} rpc={})",
						client_type_, client_location_,
						len, size, rpc_id
					);
					return {.result = -EAGAIN, .client_timestamp = remote_timestamp};
				}

				/* call the handler function */
				LOG::trace_in(client_type_,
					"[{} at '{}']: «app.name»::Protocol::handle: delegating to handler"
					" (available={} needed={} rpc={})",
					client_type_, client_location_,
					len, size, rpc_id
				);
				(record->handler_fn)(record->context, remote_timestamp.count(), (char *)dst_buffer);

				LOG::trace_in(client_type_,
					"[{} at '{}']: «app.name»::Protocol::handle: consumed {} bytes total",
					client_type_,
					client_location_,
					size + sizeof(u64)
				);
				return {.result = static_cast<int>(size + sizeof(u64)),
								.client_timestamp = remote_timestamp};
			«ENDIF»
		}

		/******************************************************************************
		 * PROTOCOL CLASS: handle_multiple
		 ******************************************************************************/
		Protocol::handle_result_t Protocol::handle_multiple(const char *msg, u64 len)
		{
			u64 processed = 0;
			u64 remaining = len;
			int ret = 0;
			u16 count = 0;
			auto client_timestamp = std::chrono::nanoseconds::zero();

			while (len > processed) {
				auto const handled = handle(msg + processed,
						(remaining > ((u32)-1) ? ((u32)-1) : remaining));
				ret = handled.result;
				client_timestamp = handled.client_timestamp;
				assert(ret != 0);
				if (ret < 0) {
					LOG::trace_in(
						client_type_, "[{} at '{}']: «app.name»::Protocol::handle_multiple:"
						" error while handling message (received={} handled={} processed={}"
						" remaining={} count={})", client_type_, client_location_,
						len, ret, processed, remaining, count
					);
					break;
				}
				assert ((u32)ret <= remaining);

				/* sanity check, should not happen */
				if (((u64)ret + processed > len) || (((u64)ret + processed) < processed)) {
					LOG::critical(
						"«app.name»::Protocol::handle_multiple: possible overflow"
						" (received={} handled={} processed={} remaining={} count={})",
						len, ret, processed, remaining, count
					);
					throw std::runtime_error("Possible overflow in handle_multiple");
				}

				processed += ret;
				remaining -= ret;
				++count;

				LOG::trace_in(
					client_type_, "[{} at '{}']: «app.name»::Protocol::handle_multiple:"
					" handled message (received={} handled={} processed={} remaining={} count={})",
					client_type_, client_location_,
					len, ret, processed, remaining, count
				);
			}

			if (processed > 0) {
				LOG::trace_in(
					client_type_, "[{} at '{}']: «app.name»::Protocol::handle_multiple:"
					" done handling at least one message (received={} handled={} processed={}"
					" remaining={} count={})", client_type_, client_location_,
					len, ret, processed, remaining, count
				);
				return {.result = static_cast<int>(processed), .client_timestamp = client_timestamp};
			}

			/* error, return code (or in edge case of len == 0, returns 0) */
			LOG::trace_in(
				client_type_, "[{} at '{}']: «app.name»::Protocol::handle_multiple:"
				" done handling no message (received={} handled={} processed={} remaining={}"
				" count={})", client_type_, client_location_,
				len, ret, processed, remaining, count
			);
			return {.result = ret, .client_timestamp = client_timestamp};
		}

		/******************************************************************************
		 * PROTOCOL CLASS: add handler
		 ******************************************************************************/
		void Protocol::add_handler(u16 rpc_id, void *context, handler_func_t handler_fn)
		{
			func_info *record = funcs_.insert(rpc_id, func_info{ context, handler_fn, });
			if (record == nullptr)
				throw std::runtime_error(fmt::format(
					"Protocol::add_handler: unable to insert handler_fn (rpc_id={})", rpc_id));
		}

		«IF app.jit»
		/******************************************************************************
		 * PROTOCOL CLASS: transform insert
		 ******************************************************************************/
		void Protocol::insert_transform(u16 rpc_id, transform xform,
				u32 size, std::shared_ptr<jitbuf::TransformRecord> &transform_ptr)
		{
			/* find our handler function */
			auto func_info_p = funcs_.find(rpc_id);
			if (func_info_p == nullptr)
				throw std::runtime_error(fmt::format(
					"Protocol::insert_transform: handler not found (rpc_id={})", rpc_id));

			handler_info *record = handlers_.insert(rpc_id, handler_info{
					.xform = xform,
					.context = func_info_p->context,
					.handler_fn = func_info_p->handler_fn,
					.size = size,
					.transform_ptr = transform_ptr });
			if (record == nullptr)
				throw std::runtime_error(fmt::format(
					"Protocol::insert_transform: unable to insert transform (rpc_id={})", rpc_id));
		}
		«ENDIF»

		/******************************************************************************
		 * PROTOCOL CLASS: insert_identity_transform with RPC ID
		 ******************************************************************************/
		void Protocol::insert_identity_transform(u16 rpc_id)
		{
			/* find our handler function */
			auto func_info_p = funcs_.find(rpc_id);
			if (func_info_p == nullptr)
				throw std::runtime_error(fmt::format(
					"Protocol::insert_identity_transform: handler not found (rpc_id={})", rpc_id));

			handler_info *record = handlers_.insert(rpc_id, handler_info{
					.xform = builder_.get_identity(rpc_id),
					.context = func_info_p->context,
					.handler_fn = func_info_p->handler_fn,
					.size = builder_.get_identity_size(rpc_id),
				«IF app.jit»
					.transform_ptr = nullptr
				«ENDIF»
			});
			if (record == nullptr)
				throw std::runtime_error(fmt::format(
					"Protocol::insert_identity_transform: unable to insert identity transform (rpc_id={})", rpc_id));
		}

		/******************************************************************************
		 * PROTOCOL CLASS: insert_no_auth_identity_transforms
		 ******************************************************************************/
		void Protocol::insert_no_auth_identity_transforms()
		{
			«FOR span : app.spans»
				«FOR msg : span.messages»
					«IF msg.noAuthorizationNeeded»
						/* «span.name»: «app.name».«msg.name» */
						insert_identity_transform(«msg.wire_msg.rpc_id»);
					«ENDIF»
				«ENDFOR»
			«ENDFOR»
		}

		/******************************************************************************
		 * PROTOCOL CLASS: insert_need_auth_identity_transforms
		 ******************************************************************************/
		void Protocol::insert_need_auth_identity_transforms()
		{
			«FOR span : app.spans»
				«FOR msg : span.messages»
					«IF !msg.noAuthorizationNeeded»
						/* «span.name»: «app.name».«msg.name» */
						insert_identity_transform(«msg.wire_msg.rpc_id»);
					«ENDIF»
				«ENDFOR»
			«ENDFOR»
		}

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */
		'''
	}
}
