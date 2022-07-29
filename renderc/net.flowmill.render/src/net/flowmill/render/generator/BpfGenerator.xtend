// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.generator

import net.flowmill.render.render.FieldTypeEnum
import net.flowmill.render.render.App
import net.flowmill.render.render.Message

import static extension net.flowmill.render.extensions.FieldExtensions.*
import static extension net.flowmill.render.extensions.MessageExtensions.*

class BpfGenerator {
	static def generateBpfH(App app)
	{
		return '''
		/*********************************************************************
		 * JITBUF GENERATED HEADER
		 * !!! generated code, do not modify !!!
		 *********************************************************************
		 *
		 * This header first includes a struct and descriptor for each
		 *    message in the .proto file, then a joint descriptor for the
		 *    entire .proto file, protected by an include guard.

		 *********************************************************************/

		#pragma once

		#include <linux/stddef.h>

		#include "jitbuf/jb.h"

		#include "generated/«app.jb_h»"

		#ifdef __cplusplus
		extern "C" {
		#endif /* __cplusplus */

		/******************************
		 * MSG FILLER FUNCTIONS
		 ******************************/
		«FOR msg : app.spans.flatMap[messages].toSet»
			«fillerFunction(msg, app)»

		«ENDFOR»


		/******************************
		 * BPF MESSAGE STRUCTS
		 ******************************/
		«FOR msg : app.spans.flatMap[messages].toSet»
			«bpfMessageStruct(msg, app)»

		«ENDFOR»

		/******************************
		 * BPF FILLER FUNCTIONS
		 ******************************/
		«FOR msg : app.spans.flatMap[messages].toSet»
			«bpfFillerFunction(msg, app)»

		«ENDFOR»
		#ifdef __cplusplus
		}
		#endif /* __cplusplus */
		'''
	}

	static def fillerFunctionName(Message msg, App app)
	{
		val jmsg = msg.wire_msg
		return  jmsg.attr_name + '_fill_' + app.name + '__' + msg.name
	}

	static def bpfStructName(Message msg, App app)
	{
		return 'bpf_' + app.name + '__' + msg.name
	}

	static def fillerFunction(Message msg, App app)
	{
		val jmsg = msg.wire_msg
		val filler_function_name = fillerFunctionName(msg, app)
		'''
		static inline void «filler_function_name»( struct «jmsg.struct_name» *msg «msg.commaPrototype»)
		{
			msg->_rpc_id = «jmsg.rpc_id»;

			/* copy all non-string fields */
			«FOR field : msg.fields.filter[type.enum_type != FieldTypeEnum.STRING]»
				«IF field.isArray»
					bpf_probe_read(msg->«field.name», «field.size(true)», «field.name»);
				«ELSE»
					msg->«field.name» = «field.name»;
				«ENDIF»
			«ENDFOR»
			«IF jmsg.dynamic_size»

				/* handle dynamic string lengths */
				u32 __consumed = «jmsg.size»;
				«FOR field : jmsg.fields.filter[type.enum_type == FieldTypeEnum.STRING]»
						/* not the last field: length is in wire message */
						msg->«field.name» = «field.name».len;
						__consumed += «field.name».len;
				«ENDFOR»
				/* last field: gets the rest of the message */
				__consumed += «jmsg.last_blob_field.name».len;

				/* check that we don't write dynamic message >= 1<<16 */
				msg->_len = (__consumed <= 0xffff) ? (u16)__consumed : (u16)0xffff;
			«ENDIF»
		}

		/* check that filler_prototype and commaCallPrototype match */
		static inline void __jitbuf_check_filler_prototype_«app.name»__«msg.name»(int __dummy «msg.commaPrototype») {
			struct «jmsg.struct_name» *__msg;
			«filler_function_name»(__msg «msg.commaCallPrototype»);
		}
		'''
	}

	static def bpfMessageStruct(Message msg, App app)
	{
		val jmsg = msg.wire_msg
		val bpf_struct_name = bpfStructName(msg, app)
		'''
		/************************************
		 * «msg.name»
		 ************************************/
		struct «bpf_struct_name» {
			u32 dummy; // needed for 64 bit aligned
		    u32 unpadded_size;
		    u64 timestamp;
		    struct «jmsg.struct_name» jb;
			u8 __pad[«(8 - (jmsg.size % 8)) % 8»];
		};
		static const uint32_t «bpf_struct_name»__data_size = «jmsg.size» + 16;
		static const uint32_t «bpf_struct_name»__perf_size = «((jmsg.size + 8 + 7) / 8) * 8 + 4»;

		/* static asserts that memory layout of message «msg.name» conforms to jitbuf's assumptions */
		#define JB_ASSERT(name, predicate)     typedef char _jitbuf_static_assert_##name[2*!!(predicate)-1];
		JB_ASSERT(«bpf_struct_name»__has_correct_offset,offsetof(struct «bpf_struct_name»,jb) == 16)
		JB_ASSERT(«bpf_struct_name»_has_correct_sizeof,((sizeof(struct «bpf_struct_name») + 1) & ~1) >= «jmsg.size»+16)
		#undef JB_ASSERT
		'''
	}

	static def bpfFillerFunction(Message msg, App app)
	{
		val jmsg = msg.wire_msg
		val bpf_struct_name = bpfStructName(msg, app)
		val filler_function_name = fillerFunctionName(msg, app)
		'''
		/* «msg.name» */
		static inline void bpf_fill_«app.name»__«msg.name»( struct «bpf_struct_name» *__msg, u64 __now  «msg.commaPrototype»)
		{
			__msg->timestamp = __now;
			«filler_function_name»(&__msg->jb «msg.commaCallPrototype»);
			«IF jmsg.dynamic_size»
				__msg->unpadded_size = 8 + __msg->jb._len;
			«ELSE»
				__msg->unpadded_size = «jmsg.size + 8»;
			«ENDIF»
		}

		/* check that filler_prototype and commaCallPrototype match */
		static inline void __jitbuf_check_bpf_filler_prototype_«app.name»_«msg.name»(int __dummy «msg.commaPrototype») {
			struct «bpf_struct_name» *__msg;
			u64 __now;
			bpf_fill_«app.name»__«msg.name»(__msg, __now «msg.commaCallPrototype»);
		}

		static inline int perf_submit_«app.name»__«msg.name»(struct pt_regs *ctx,
			u64 __now «msg.commaPrototype»)
		{
			struct «bpf_struct_name» __msg = {};
			bpf_fill_«app.name»__«msg.name»(&__msg, __now «msg.commaCallPrototype»);
			return events.perf_submit(ctx, &__msg.unpadded_size, «bpf_struct_name»__perf_size);
		}
		'''
	}
}
