// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import java.util.List

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.Message
import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.XPackedMessageExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*

/**
 * Generates message-related code (previously "jitbuf")
 */
class MessageGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
    val messages = app.messages

    fsa.generateFile(outputPath(app, "wire_message.h"), generateMessageH(messages, true))
    fsa.generateFile(outputPath(app, "parsed_message.h"), generateMessageH(messages, false))

    fsa.generateFile(outputPath(app, "descriptor.h"), generateDescriptorH(messages))
    fsa.generateFile(outputPath(app, "descriptor.cc"), generateDescriptorCc(messages))

    fsa.generateFile(outputPath(app, "meta.h"), generateMetaH(app, messages))
  }

  static def generateMessageH(List<Message> messages, boolean wire_message) {
    '''
    «generatedCodeWarning()»
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

    «FOR msg : messages»
    «val xmsg = if (wire_message) msg.wire_msg else msg.parsed_msg»
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
    #ifdef __cplusplus
        #define JB_ASSERT(name, predicate) static_assert(predicate, #name ": " #predicate)
    #else
        #define JB_ASSERT(name, predicate) _Static_assert(predicate, #name ": " #predicate)
    #endif
    «FOR field : xmsg.fields»
      JB_ASSERT(«xmsg.struct_name»_«field.name»_has_correct_offset,offsetof(struct «xmsg.struct_name»,«field.name») == «if (wire_message) field.wire_pos else field.parsed_pos»);
    «ENDFOR»
    JB_ASSERT(«xmsg.struct_name»_has_correct_sizeof,((sizeof(struct «xmsg.struct_name») + 1) & ~1) >= «xmsg.size»);
    #undef JB_ASSERT

    #define «xmsg.struct_name»__rpc_id    «xmsg.rpc_id»

    «ENDFOR»
    '''
  }

  static def generateDescriptorH(List<Message> messages) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include <stddef.h>
    #include <string>

    «FOR msg : messages»
      /* JitbufDescriptor for message «msg.name» */
      extern const std::string «msg.wire_msg.descriptor_name»;
      /* JitbufExtDescriptor for message «msg.name» */
      extern const std::string «msg.parsed_msg.descriptor_name»;
    «ENDFOR»
    '''
  }

  static def generateDescriptorCc(List<Message> messages) {
    '''
    «generatedCodeWarning()»

    #include "descriptor.h"
    #include <cstdint>

    /***********************
     * DESCRIPTORS
     ***********************/
    «FOR msg : messages»
      «FOR xmsg : List.of(msg.wire_msg, msg.parsed_msg)»
        static const uint16_t «xmsg.descriptor_name»_buffer[] = {«xmsg.descriptor.map[toString].join(',')»};
        const std::string «xmsg.descriptor_name»((const char*)«xmsg.descriptor_name»_buffer, «xmsg.descriptor.size * 2»);
      «ENDFOR»
    «ENDFOR»
    '''
  }

  static def generateMetaH(App app, List<Message> messages) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include "parsed_message.h"
    #include "wire_message.h"
    #include "protocol.h"
    #include "transform_builder.h"

    #include <util/meta.h>

    #include <string_view>
    #include <type_traits>

    #include <cstdint>

    namespace «app.pkg.name» { /* pkg */
    namespace «app.name» { /* app */

    «FOR msg : messages»
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
    } // namespace «app.name» /* app */

    class «app.name»_metadata {

      «FOR msg : messages»
        static «app.name»::«msg.name»_message_metadata message_metadata_for_impl(
            «msg.wire_msg.struct_name» const &);
        static «app.name»::«msg.name»_message_metadata message_metadata_for_impl(
            «msg.parsed_msg.struct_name» const &);
      «ENDFOR»

    public:
      using protocol = «app.name»::Protocol;
      using transform_builder = «app.name»::TransformBuilder;

      using messages = meta::list<«FOR msg : messages SEPARATOR ", "»«app.name»::«msg.name»_message_metadata«ENDFOR»>;

      template <typename MessageStruct>
      using message_metadata_for = decltype(
        message_metadata_for_impl(std::declval<MessageStruct>())
      );
    };

    } // namespace «app.pkg.name» /* pkg */
    '''
  }

}
