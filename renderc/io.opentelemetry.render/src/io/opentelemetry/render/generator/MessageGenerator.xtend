// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import java.util.Collections
import io.opentelemetry.render.render.App
import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.IFileSystemAccess2

import static extension io.opentelemetry.render.extensions.XPackedMessageExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*

/**
 * Generates message-related code (previously "jitbuf")
 */
class MessageGenerator {

	static def void doGenerate(Resource resource, IFileSystemAccess2 fsa,
		String pkg_name)
	{

		for (app : resource.allContents.filter(App).toIterable) {
			val app_path = pkg_name + '/' + app.name

			fsa.generateFile(app_path + ".wire_message.h", generateMessageH(app, true))
			fsa.generateFile(app_path + ".parsed_message.h", generateMessageH(app, false))
			fsa.generateFile(app_path + ".descriptor.h", generateDescriptorH(app))
			fsa.generateFile(app_path + ".descriptor.cc", generateDescriptorCc(app))
		}
	}

	static def generateMessageH(App app, boolean wire_message) {
		return '''
		/*********************************************************************
		 * JITBUF GENERATED HEADER
		 * !!! generated code, do not modify !!!
		 *********************************************************************/

		#pragma once

		#ifdef KBUILD_MODNAME
		# include <linux/stddef.h>
		#else
		# include <stddef.h>
		#endif
		#include "jitbuf/jb.h"

		#ifdef __cplusplus
		# include <util/raw_json.h>

		# include <utility>
		#endif /* __cplusplus */

		«FOR span : app.spans»
		«FOR msg : span.messages»
		«FOR xmsg: Collections.singletonList(msg).map[if (wire_message) wire_msg else parsed_msg]»
		/************************************
		 * «msg.name»
		 ************************************/
		#ifdef __cplusplus
		extern "C" {
		#endif /* __cplusplus */
		struct «xmsg.struct_name» {
			uint16_t _rpc_id;
			«IF wire_message && xmsg.dynamic_size»
				uint16_t _len;
			«ENDIF»
			«FOR field : xmsg.fields»
				«IF field.type.isShortString»
					char «field.name»[«field.type.size»]«field.arraySuffix»;
				«ELSE»
					«xmsg.cType(field.type)» «field.name»«field.arraySuffix»;
				«ENDIF»
			«ENDFOR»

		#ifdef __cplusplus
			void dump_json(std::ostream &out) const {
				out << "\"@msg\":\"«xmsg.struct_name»\"";
				«FOR field: xmsg.fields»
					print_json_value(out << ",\"«field.name»\":", «field.name»);
				«ENDFOR»
				out << '}';
			}
		#endif /* __cplusplus */
		};
		static const uint32_t «xmsg.struct_name»__data_size = «xmsg.size»;
		#ifdef __cplusplus
		} /* extern "C" */

		template <typename Out>
		Out &&operator <<(Out &&out, «xmsg.struct_name» const &what) {
			what.dump_json(out);
			return std::forward<Out>(out);
		}
		#endif /* __cplusplus */

		/* static asserts that memory layout of message «msg.name» conforms to jitbuf's assumptions */
		#define JB_ASSERT(name, predicate) typedef char _jitbuf_static_assert_##name[2*!!(predicate)-1];
		«FOR field : xmsg.fields»
			JB_ASSERT(«xmsg.struct_name»_«field.name»_has_correct_offset,offsetof(struct «xmsg.struct_name»,«field.name») == «if (wire_message) field.wire_pos else field.parsed_pos»)
		«ENDFOR»
		JB_ASSERT(«xmsg.struct_name»_has_correct_sizeof,((sizeof(struct «xmsg.struct_name») + 1) & ~1) >= «xmsg.size»)
		#undef JB_ASSERT

		#define «xmsg.struct_name»__rpc_id		«xmsg.rpc_id»

		«ENDFOR»
		«ENDFOR»
		«ENDFOR»
		'''
	}

	static def generateDescriptorH(App app) {
		return '''
		#pragma once
		#include <stddef.h>
		#include <string>

		«FOR span : app.spans»
		«FOR msg : span.messages»
			/* JitbufDescriptor for message «msg.name» */
			extern const std::string «msg.wire_msg.descriptor_name»;
			/* JitbufExtDescriptor for message «msg.name» */
			extern const std::string «msg.parsed_msg.descriptor_name»;
		«ENDFOR»
		«ENDFOR»
		'''
	}

	static def generateDescriptorCc(App app) {
		'''
		#include "«app.descriptor_h»"

		/***********************
		 * DESCRIPTORS
		 ***********************/
		«FOR span : app.spans»
		«FOR xmsg : span.messages.map[wire_msg]»
			static const uint16_t «xmsg.descriptor_name»_buffer[] = {«xmsg.descriptor.map[toString].join(',')»};
			const std::string «xmsg.descriptor_name»((const char*)«xmsg.descriptor_name»_buffer, «xmsg.descriptor.size * 2»);
		«ENDFOR»
		«FOR xmsg : span.messages.map[parsed_msg]»
			static const uint16_t «xmsg.descriptor_name»_buffer[] = {«xmsg.descriptor.map[toString].join(',')»};
			const std::string «xmsg.descriptor_name»((const char*)«xmsg.descriptor_name»_buffer, «xmsg.descriptor.size * 2»);
		«ENDFOR»
		«ENDFOR»
		'''
	}

}
