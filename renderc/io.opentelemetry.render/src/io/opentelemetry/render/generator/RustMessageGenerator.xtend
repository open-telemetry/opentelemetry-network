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
 * Generates both wire_messages.rs and parsed_message.rs for every app.
 * - wire_messages.rs: #[repr(C)] wire structs mirroring C++ wire_message.h,
 *   with simple layout tests and metadata() for render_parser.
 * - parsed_message.rs: ergonomic parsed structs (message-name identifiers)
 *   plus decode() from a wire body slice into owned Strings for dynamic fields.
 */
class RustMessageGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
    fsa.generateFile(outputPath(app, "src/wire_messages.rs"), generateWireMessages(app))
    fsa.generateFile(outputPath(app, "src/parsed_message.rs"), generateParsedMessages(app))
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

      impl «xmsg.struct_name» {
        #[inline]
        pub fn metadata() -> render_parser::MessageMetadata {
          «IF xmsg.dynamic_size»
            render_parser::MessageMetadata::new_dynamic(«xmsg.rpc_id»u16, «!msg.noAuthorizationNeeded»)
          «ELSE»
            render_parser::MessageMetadata::new_fixed(«xmsg.rpc_id»u16, «xmsg.size», «!msg.noAuthorizationNeeded»)
          «ENDIF»
        }
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

    #[inline]
    pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
      ::std::vec![
        «FOR msg : app.messages»
          «msg.wire_msg.struct_name»::metadata(),
        «ENDFOR»
      ]
    }

    '''
  }

  private static def generateParsedMessages(App app) {
    // Validate unsupported features at generation-time: arrays of dynamic strings
    if (app.messages.exists[m | m.fields.exists[f | f.isArray && f.type.enum_type == FieldTypeEnum.STRING]]) {
      throw new RuntimeException("arrays of dynamic strings are not supported in Rust parsed decode")
    }

    '''
    «generatedCodeWarning()»
    #[allow(non_camel_case_types)]
    #[allow(non_snake_case)]
    #[allow(dead_code)]

    // For slice -> array conversions in from_ne_bytes calls
    #[allow(unused_imports)]
    use core::convert::TryInto;

    #[derive(Debug, Clone, PartialEq, Eq)]
    pub enum DecodeError {
      BufferTooSmall,
      InvalidRpcId { got: u16 },
      InvalidLength { len: u16 },
      Utf8 { field: &'static str },
    }

    «FOR msg : app.messages»
      «val w = msg.wire_msg»
      // Parsed struct for «msg.name»
      pub struct «msg.name» {
        pub _rpc_id: u16,
        «FOR field : msg.fields.sortBy[id]»
          «IF field.type.isShortString && field.isArray»
            pub «field.name»: [[u8; «field.type.size»]; «field.array_size»],
          «ELSEIF field.type.isShortString»
            pub «field.name»: [u8; «field.type.size»],
          «ELSEIF field.type.enum_type == FieldTypeEnum.STRING»
            pub «field.name»: ::std::string::String,
          «ELSEIF field.isArray»
            pub «field.name»: [«rustScalarType(field)»; «field.array_size»],
          «ELSE»
            pub «field.name»: «rustScalarType(field)»,
          «ENDIF»
        «ENDFOR»
      }

      impl «msg.name» {
        pub const RPC_ID: u16 = «w.rpc_id»u16;

        #[inline]
        pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
          // Require rpc_id
          if body.len() < 2 { return Err(DecodeError::BufferTooSmall); }
          let mut b2 = [0u8;2];
          b2.copy_from_slice(&body[0..2]);
          let rpc = u16::from_ne_bytes(b2);
          if rpc != Self::RPC_ID { return Err(DecodeError::InvalidRpcId { got: rpc }); }

          «IF w.dynamic_size»
            if body.len() < 4 { return Err(DecodeError::BufferTooSmall); }
            let mut b2 = [0u8;2];
            b2.copy_from_slice(&body[2..4]);
            let __len = u16::from_ne_bytes(b2);
            if __len < 4 { return Err(DecodeError::InvalidLength { len: __len }); }
            if body.len() < __len as usize { return Err(DecodeError::BufferTooSmall); }
          «ELSE»
            if body.len() < «w.size»usize { return Err(DecodeError::BufferTooSmall); }
          «ENDIF»

          // Decode fixed header fields
          «FOR field : msg.fields.sortBy[id]»
            «IF field.type.enum_type == FieldTypeEnum.STRING»
              // dynamic string; decode later from payload
            «ELSEIF field.type.isShortString && field.isArray»
              let «field.name» = {
                let mut tmp = [[0u8; «field.type.size»]; «field.array_size»];
                let elem = «field.type.size»usize;
                let mut i = 0usize;
                while i < «field.array_size»usize {
                  let off = «field.wire_pos»usize + i * elem;
                  tmp[i].copy_from_slice(&body[off .. off + elem]);
                  i += 1;
                }
                tmp
              };
            «ELSEIF field.type.isShortString»
              let «field.name» = {
                let mut tmp = [0u8; «field.type.size»];
                let off = «field.wire_pos»usize;
                tmp.copy_from_slice(&body[off .. off + «field.type.size»usize]);
                tmp
              };
            «ELSEIF field.isArray»
              let «field.name» = {
                let mut tmp = [«defaultValue(field)»; «field.array_size»];
                let es = «elemSize(field)»usize;
                let mut i = 0usize;
                while i < «field.array_size»usize {
                  let off = «field.wire_pos»usize + i * es;
                  tmp[i] = «readScalar(field, 'off')»;
                  i += 1;
                }
                tmp
              };
            «ELSE»
              let «field.name» = «readScalar(field, field.wire_pos + "usize")»;
            «ENDIF»
          «ENDFOR»

          // Decode dynamic payload strings
          «IF w.dynamic_size»
            let mut __off = «w.size»usize;
            «FOR field : msg.fields.filter[type.enum_type == FieldTypeEnum.STRING].sortBy[id]»
              «IF field != w.last_blob_field»
                // length from header
                let mut __b = [0u8;2];
                __b.copy_from_slice(&body[«field.wire_pos»usize .. «field.wire_pos»usize + 2]);
                let __l_«field.name» = u16::from_ne_bytes(__b) as usize;
                if __off + __l_«field.name» > body.len() { return Err(DecodeError::BufferTooSmall); }
                let «field.name» = if __l_«field.name» == 0 { ::std::string::String::new() } else { ::std::string::String::from_utf8_lossy(&body[__off .. __off + __l_«field.name»]).into_owned() };
                __off += __l_«field.name»;
              «ELSE»
                let __tail = (__len as usize).saturating_sub(__off);
                if __off + __tail > body.len() { return Err(DecodeError::BufferTooSmall); }
                let «field.name» = if __tail == 0 { ::std::string::String::new() } else { ::std::string::String::from_utf8_lossy(&body[__off .. __off + __tail]).into_owned() };
              «ENDIF»
            «ENDFOR»
          «ENDIF»

          Ok(Self {
            _rpc_id: rpc,
            «FOR field : msg.fields.sortBy[id] SEPARATOR ","»
              «field.name»: «field.name»
            «ENDFOR»
          })
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
      case FieldTypeEnum.STRING: 'u16' // not used for parsed; handled separately
    }
  }

  private static def String defaultValue(Field field) {
    switch (field.type.enum_type) {
      case FieldTypeEnum.U8: '0u8'
      case FieldTypeEnum.U16: '0u16'
      case FieldTypeEnum.U32: '0u32'
      case FieldTypeEnum.U64: '0u64'
      case FieldTypeEnum.U128: '0u128'
      case FieldTypeEnum.S8: '0i8'
      case FieldTypeEnum.S16: '0i16'
      case FieldTypeEnum.S32: '0i32'
      case FieldTypeEnum.S64: '0i64'
      case FieldTypeEnum.S128: '0i128'
      case FieldTypeEnum.STRING: '0u16'
    }
  }

  private static def String elemSize(Field field) {
    // Element size in bytes for array elements in the wire header
    if (field.type.isShortString) {
      return field.type.size.toString
    }
    switch (field.type.enum_type) {
      case FieldTypeEnum.U8,
      case FieldTypeEnum.S8: '1'
      case FieldTypeEnum.U16,
      case FieldTypeEnum.S16: '2'
      case FieldTypeEnum.U32,
      case FieldTypeEnum.S32: '4'
      case FieldTypeEnum.U64,
      case FieldTypeEnum.S64: '8'
      case FieldTypeEnum.U128,
      case FieldTypeEnum.S128: '16'
      case FieldTypeEnum.STRING: '2'
    }
  }

  private static def String readScalar(Field field, Object offExpr) {
    // Generates an expression to read a scalar of the field's element type from body at offset offExpr
    switch (field.type.enum_type) {
      case FieldTypeEnum.U8: '''body[«offExpr»]'''
      case FieldTypeEnum.S8: '''(body[«offExpr»] as i8)'''
      case FieldTypeEnum.U16: '''u16::from_ne_bytes(body[«offExpr» .. «offExpr» + 2].try_into().unwrap())'''
      case FieldTypeEnum.S16: '''i16::from_ne_bytes(body[«offExpr» .. «offExpr» + 2].try_into().unwrap())'''
      case FieldTypeEnum.U32: '''u32::from_ne_bytes(body[«offExpr» .. «offExpr» + 4].try_into().unwrap())'''
      case FieldTypeEnum.S32: '''i32::from_ne_bytes(body[«offExpr» .. «offExpr» + 4].try_into().unwrap())'''
      case FieldTypeEnum.U64: '''u64::from_ne_bytes(body[«offExpr» .. «offExpr» + 8].try_into().unwrap())'''
      case FieldTypeEnum.S64: '''i64::from_ne_bytes(body[«offExpr» .. «offExpr» + 8].try_into().unwrap())'''
      case FieldTypeEnum.U128: '''u128::from_ne_bytes(body[«offExpr» .. «offExpr» + 16].try_into().unwrap())'''
      case FieldTypeEnum.S128: '''i128::from_ne_bytes(body[«offExpr» .. «offExpr» + 16].try_into().unwrap())'''
      case FieldTypeEnum.STRING: '''u16::from_ne_bytes(body[«offExpr» .. «offExpr» + 2].try_into().unwrap())'''
    }
  }
}
