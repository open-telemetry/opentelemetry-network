// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.FieldTypeEnum
import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*

class WriterGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
      fsa.generateFile(outputPath(app, "writer.h"), generateWriterH(app))
      fsa.generateFile(outputPath(app, "writer.cc"), generateWriterCc(app))

      fsa.generateFile(outputPath(app, "encoder.h"), generateEncoderH(app))
      fsa.generateFile(outputPath(app, "encoder.cc"), generateEncoderCc(app))
  }

  private static def generateWriterH(App app) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include "encoder.h"
    #include "parsed_message.h"
    #include "wire_message.h"

    #include <channel/ibuffered_writer.h>
    #include <platform/types.h>

    #include <functional>
    #include <stdexcept>
    #include <system_error>
    #include <utility>

    namespace «app.pkg.name»::«app.name» {

    class Writer {
    public:
      using clock_t = std::function<u64()>;

      Writer(IBufferedWriter &buffer, clock_t clock, u64 time_adjustment = 0, Encoder *encoder = nullptr);

      Writer(Writer const &);
      Writer(Writer &&);

      Writer &operator =(Writer const &) = delete;
      Writer &operator =(Writer &&) = delete;

      «FOR msg : app.messages»

        /* «msg.name» */
        template <bool ThrowOnError = true>
        inline std::error_code «msg.name»(«msg.prototype») {
          return «msg.name»_tstamp<ThrowOnError>(clock_()+time_adjustment_«msg.commaCallPrototype»);
        }

        template <bool ThrowOnError = true>
        inline std::error_code «msg.name»_tstamp(u64 __tstamp«msg.commaPrototype») {
          auto __result = encoder_->«msg.name»(buffer_, __tstamp«msg.commaCallPrototype»);

          if constexpr (ThrowOnError) {
            if (__result) {
              throw std::system_error(__result, "Writer::«msg.name» failed");
            }
          }

          return __result;
        }
      «ENDFOR»

      bool is_writable() const {
        return buffer_.is_writable();
      }

      std::error_code flush() {
        if (is_writable()) {
          return buffer_.flush();
        }
        return {};
      }

    private:
      IBufferedWriter &buffer_;
      Encoder default_encoder_;
      Encoder *encoder_ = nullptr;
      clock_t clock_;
      u64 time_adjustment_;
    };

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def generateWriterCc(App app) {
    '''
    «generatedCodeWarning()»

    #include "writer.h"

    namespace «app.pkg.name»::«app.name» {

    Writer::Writer(IBufferedWriter &buffer, clock_t clock, u64 time_adjustment, Encoder *encoder):
      buffer_(buffer),
      encoder_(encoder ? encoder : &default_encoder_),
      clock_(std::move(clock)),
      time_adjustment_(time_adjustment)
    {}

    Writer::Writer(Writer const &rhs):
      buffer_(rhs.buffer_),
      encoder_(rhs.encoder_ == &rhs.default_encoder_ ? &default_encoder_ : rhs.encoder_),
      clock_(rhs.clock_),
      time_adjustment_(rhs.time_adjustment_)
    {}

    Writer::Writer(Writer &&rhs):
      buffer_(rhs.buffer_),
      encoder_(rhs.encoder_ == &rhs.default_encoder_ ? &default_encoder_ : rhs.encoder_),
      clock_(std::move(rhs.clock_)),
      time_adjustment_(std::move(rhs.time_adjustment_))
    {}

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def generateEncoderH(App app) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include "parsed_message.h"
    #include "wire_message.h"

    #include <channel/ibuffered_writer.h>
    #include <platform/types.h>

    namespace «app.pkg.name»::«app.name» {

    class Encoder {
    public:
      Encoder();
      virtual ~Encoder();

      «FOR msg : app.messages»
      virtual std::error_code «msg.name»(IBufferedWriter &__buffer, u64 __tstamp«msg.commaPrototype»);
      «ENDFOR»
    };

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def generateEncoderCc(App app) {
    '''
    «generatedCodeWarning()»

    #include "encoder.h"
    #include "wire_message.h"

    #include <jitbuf/jb.h>
    #include <platform/types.h>

    #include <cassert>
    #include <cstring>
    #include <stdexcept>
    #include <system_error>

    namespace «app.pkg.name»::«app.name» {

    «FOR msg : app.messages»
      extern "C" void «app.pkg.name»_«app.name»_encode_«msg.name»(
          uint8_t *__dest,
          uint32_t __dest_len,
          uint64_t __tstamp«FOR field : msg.fields.sortBy[id] BEFORE ',' SEPARATOR ','»
            «ffiParamC(field)»
          «ENDFOR»
      );
    «ENDFOR»

    Encoder::Encoder() {}
    Encoder::~Encoder() {}

    «FOR msg : app.messages»
      /* «msg.name» via Rust shim */
      std::error_code Encoder::«msg.name»(IBufferedWriter &__buffer, u64 __tstamp«msg.commaPrototype») {
        /* Compute encoded length (wire struct + dynamic payloads) */
        u32 __consumed = «msg.wire_msg.size»;
        «IF msg.wire_msg.dynamic_size»
          /* handle dynamic string lengths */
          «FOR field : msg.wire_msg.fields.filter[type.enum_type == FieldTypeEnum.STRING]»
            __consumed += «field.name».len;
          «ENDFOR»
          __consumed += «msg.wire_msg.last_blob_field.name».len;
          if (__consumed > 0xffff) {
            throw std::runtime_error("Writer::«msg.name» tried to write dynamic message >= 1<<16");
          }
        «ENDIF»

        /* start write */
        const u32 __total_len = sizeof(u64) + __consumed;
        auto __allocated = __buffer.start_write(__total_len);
        if (!__allocated) {
          return __allocated.error();
        }

        /* Call Rust encoder */
        «app.pkg.name»_«app.name»_encode_«msg.name»(
            (uint8_t *)*__allocated,
            __total_len,
            __tstamp«FOR field : msg.fields.sortBy[id] BEFORE ',' SEPARATOR ','»
              «ffiCallArg(field)»
            «ENDFOR»
        );

        __buffer.finish_write();
        return {};
      }
    «ENDFOR»

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def String ffiParamC(io.opentelemetry.render.render.Field field) {
    if (field.type.enum_type == FieldTypeEnum.STRING) {
      return '''struct jb_blob «field.name»'''
    }

    if (field.type.isShortString || field.isArray) {
      val elemC =
        if (field.type.isShortString) 'uint8_t' else field.type.cType(true)
      return '''const «elemC» *«field.name»'''
    }

    return '''«field.type.cType(false)» «field.name»'''
  }

  private static def String ffiCallArg(io.opentelemetry.render.render.Field field) {
    if (field.type.enum_type == FieldTypeEnum.STRING) {
      return field.name
    }
    if (field.type.isShortString) {
      return '''(const uint8_t *)&«field.name»[0]'''
    }
    if (field.isArray) {
      return '''&«field.name»[0]'''
    }
    return field.name
  }

}
