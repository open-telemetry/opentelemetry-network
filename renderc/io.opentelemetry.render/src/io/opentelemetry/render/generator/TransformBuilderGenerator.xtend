// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import io.opentelemetry.render.render.App
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.SpanExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*
import io.opentelemetry.render.render.Message
import io.opentelemetry.render.render.FieldTypeEnum

class TransformBuilderGenerator {

	static def generateTransformerH(App app, String pkg_name)
	{
		'''
		#pragma once

		#include <stdexcept>
		#include <jitbuf/perfect_hash.h>
		#include <generated/«pkg_name»/«app.name»/hash.h>
		#include <platform/types.h>
		«IF app.jit»
			#include <jitbuf/transform_builder.h>
		«ENDIF»

		namespace «app.pkg.name» {
		namespace «app.name» {
		/******************************************************************************
		 * TRANSFORM BUILDER
		 ******************************************************************************/
		class TransformBuilder
				«IF app.jit»
					: public ::jitbuf::TransformBuilder
				«ENDIF»
		{
		public:
			/* message format transform function type */
			typedef uint16_t (*transform)(const char *src, char *dst);

			/**
			 * C'tor
			 */
			«IF app.jit»
				TransformBuilder(llvm::LLVMContext &context);
			«ELSE»
				TransformBuilder();
			«ENDIF»

			/**
			 * Get the size of the identity wire message for the given RPC ID
			 */
			u32 get_identity_size(u16 rpc_id);

			/**
			 * Get identity transform for the given RPC ID
			 */
			transform get_identity(u16 rpc_id);

		private:
			/* hash function for PerfectHash objects */
			struct rpc_id_hash_fn {
				u32 operator()(u32 key) { return «app.hashName»(key); }
			};

			/* information about our implemented messages */
			struct xform_info {
				transform xform;
				u16 size;
			};

			PerfectHash<xform_info, «app.hashSize», rpc_id_hash_fn> identity_xforms_;
		};

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */
		'''
	}

	static def generateTransformerCc(App app)
	{
 		'''
		#include "transform_builder.h"

		/* message structs for decoding */
		#include "generated/«app.jsrv_h»"
		#include "generated/«app.jb_h»"
		#include "generated/«app.descriptor_h»"

		/******************************************************************************
		 * IDENTITY TRANSFORM IMPLEMENTATIONS
		 ******************************************************************************/
		«FOR msg : app.spans.flatMap[messages].toSet»
			«identityTransform(msg)»

		«ENDFOR»

		namespace «app.pkg.name» {
		namespace «app.name» {

		typedef uint16_t (*transform)(const char *src, char *dst);

		/******************************************************************************
		 * TRANSFORM BUILDER: c'tor
		 ******************************************************************************/
		«IF app.jit»
			TransformBuilder::TransformBuilder(llvm::LLVMContext &context)
			  : jitbuf::TransformBuilder(context)
			{
				/* add all local message descriptors for jit */
				«FOR span : app.spans»
					«FOR msg : span.messages»
						add_descriptor(«msg.parsed_msg.descriptor_name»);
					«ENDFOR»
				«ENDFOR»

		«ELSE»
			TransformBuilder::TransformBuilder()
			{
		«ENDIF»
			/* add identity transforms */
			«FOR msg : app.spans.flatMap[messages].toSet»
				identity_xforms_.insert(«msg.parsed_msg.rpc_id»,
					xform_info{.xform = «msg.identityTransformName»,
								.size = «msg.wire_msg.size»});
			«ENDFOR»
		}

		/******************************************************************************
		 * TRANSFORM BUILDER: get identity message size
		 ******************************************************************************/
		u32 TransformBuilder::get_identity_size(u16 rpc_id)
		{
			/* find our handler function */
			auto func_info_p = identity_xforms_.find(rpc_id);
			if (func_info_p == nullptr)
				throw std::runtime_error("identity_size: rpc_id not found");
			return func_info_p->size;
		}

		/******************************************************************************
		 * TRANSFORM BUILDER: get identity transform
		 ******************************************************************************/
		transform TransformBuilder::get_identity(u16 rpc_id)
		{
			/* find our handler function */
			auto func_info_p = identity_xforms_.find(rpc_id);
			if (func_info_p == nullptr)
				throw std::runtime_error("get_identity: rpc_id not found");
			return func_info_p->xform;
		}

		} /* namespace «app.name» */
		} /* namespace «app.pkg.name» */
		'''
	}

	static def identityTransformName(Message msg) {
		val app = msg.span.app
		'''«app.c_name»_«msg.name»_identity_handler'''
	}

	static def identityTransform(Message msg) {
		'''
		static uint16_t «identityTransformName(msg)»(const char *src, char *dst) {
			«IF !msg.wire_msg.dynamic_size»
				/* simple message -- just needs a copy */
				memcpy(dst, src, «msg.wire_msg.size»);
				return «msg.wire_msg.size»;
			«ELSE»
				/* dynamic-size message */
				auto src_msg = (const «msg.wire_msg.struct_name» *)src;
				auto dst_msg = («msg.parsed_msg.struct_name» *)dst;

				/* copy all non-string fields */
				«FOR field : msg.fields.filter[type.enum_type != FieldTypeEnum.STRING]»
					«IF field.isArray»
						memcpy(&dst_msg->«field.name»[0], &src_msg->«field.name»[0], «field.size(true)»);
					«ELSE»
						dst_msg->«field.name» = src_msg->«field.name»;
					«ENDIF»
				«ENDFOR»

				/* handle dynamic strings */
				u16 consumed = «msg.wire_msg.size»;
				«FOR field : msg.fields.filter[type.enum_type == FieldTypeEnum.STRING]»
					dst_msg->«field.name».buf = &src[consumed];
					«IF field != msg.wire_msg.last_blob_field»
						/* not the last field: length is in original message */
						dst_msg->«field.name».len = src_msg->«field.name»;
						consumed += src_msg->«field.name»;
					«ELSE»
						/* last field: gets the rest of the message */
						dst_msg->«field.name».len = src_msg->_len - consumed;
					«ENDIF»
				«ENDFOR»

				dst_msg->_rpc_id = «msg.parsed_msg.rpc_id»;

				return src_msg->_len;
			«ENDIF»
		}
		'''
	}
}
