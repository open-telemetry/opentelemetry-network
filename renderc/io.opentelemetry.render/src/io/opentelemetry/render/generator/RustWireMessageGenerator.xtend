// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.FieldTypeEnum

import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*

/**
 * Generates wire_messages.rs for every app.
 * Contains #[repr(C)] wire structs mirroring C++ wire_message.h and simple
 * runtime layout tests using assert! and core::mem::{size_of, align_of, offset_of}.
 */
class RustWireMessageGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
    fsa.generateFile(outputPath(app, "wire_messages.rs"), generateWireMessages(app))
  }

  private static def generateWireMessages(App app) {
    '''
    «generatedCodeWarning()»
    #[allow(non_camel_case_types)]
    #[allow(non_snake_case)]
    #[allow(dead_code)]

    «FOR msg : app.messages»
    «val xmsg = msg.wire_msg»
      #[repr(C)]
      #[derive(Copy, Clone)]
      pub struct «xmsg.struct_name» {
        pub _rpc_id: u16,
        «IF xmsg.dynamic_size»
        pub _len: u16,
        «ENDIF»
        «FOR field : xmsg.fields»
          «IF field.isArray && field.type.isShortString»
            pub «field.name»: [[u8; «field.type.size»]; «field.array_size»],
          «ELSEIF field.type.isShortString»
            pub «field.name»: [u8; «field.type.size»],
          «ELSEIF field.isArray»
            pub «field.name»: [«rustScalarType(field)»; «field.array_size»],
          «ELSE»
            pub «field.name»: «rustScalarType(field)»,
          «ENDIF»
        «ENDFOR»
      }

      impl Default for «xmsg.struct_name» {
        #[inline]
        fn default() -> Self { unsafe { core::mem::zeroed() } }
      }

      pub const «msg.name.toUpperCase»_WIRE_SIZE: usize = «xmsg.size»;

      #[cfg(test)]
      mod «msg.name»_layout_tests {
        use super::*;
        use core::mem::{offset_of, align_of};
        #[test]
        fn struct_size() {
          let size = size_of::<«xmsg.struct_name»>();
          let align = align_of::<«xmsg.struct_name»>();
          let padded_raw_size = («msg.name.toUpperCase»_WIRE_SIZE + align - 1) / align * align;
          assert_eq!(size, padded_raw_size);
        }
        #[test]
        fn field_offsets() {
          assert_eq!(offset_of!(«xmsg.struct_name», _rpc_id), 0);
          «IF xmsg.dynamic_size»
            assert_eq!(offset_of!(«xmsg.struct_name», _len), 2);
          «ENDIF»
          «FOR field : xmsg.fields»
            assert_eq!(offset_of!(«xmsg.struct_name», «field.name»), «field.wire_pos»usize);
          «ENDFOR»
        }
      }
    «ENDFOR»

    '''
  }

  private static def String rustScalarType(Field field) {
    switch (field.type.enum_type) {
      case FieldTypeEnum.U8: 'u8'
      case FieldTypeEnum.U16: 'u16'
      case FieldTypeEnum.U32: 'u32'
      case FieldTypeEnum.U64: 'u64'
      case FieldTypeEnum.U128: 'u128'
      case FieldTypeEnum.S8: 'i8'
      case FieldTypeEnum.S16: 'i16'
      case FieldTypeEnum.S32: 'i32'
      case FieldTypeEnum.S64: 'i64'
      case FieldTypeEnum.S128: 'i128'
      case FieldTypeEnum.STRING: 'u16' // length of string in wire struct
    }
  }
}
