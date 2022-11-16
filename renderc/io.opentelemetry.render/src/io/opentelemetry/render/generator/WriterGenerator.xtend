// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.FieldTypeEnum
import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*
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

    namespace «app.pkg.name»::«app.name» {

    Encoder::Encoder() {}
    Encoder::~Encoder() {}

    «FOR msg : app.messages»
      /* «msg.name» */
      std::error_code Encoder::«msg.name»(IBufferedWriter &__buffer, u64 __tstamp«msg.commaPrototype») {
        /* allocate space on the stack, 64-bit aligned */
        /* zero out buffer so we don't exfiltrate uninitialized memory*/
        u64 __buf64[1 + (sizeof(struct «msg.wire_msg.struct_name») + 7) / 8] = {};
        u32 __len = «msg.wire_msg.size» + sizeof(u64);

        __buf64[0] = __tstamp;

        auto __dst_msg = («msg.wire_msg.struct_name» *)&__buf64[1];

        __dst_msg->_rpc_id = «msg.wire_msg.rpc_id»;

        /* copy all non-string fields */
        «FOR field : msg.fields.filter[type.enum_type != FieldTypeEnum.STRING || type.isShortString]»
          «IF field.isArray || field.type.isShortString»
            memcpy(&__dst_msg->«field.name»[0], &«field.name»[0], «field.size(true)»);
          «ELSE»
            __dst_msg->«field.name» = «field.name»;
          «ENDIF»
        «ENDFOR»

        u32 __consumed = «msg.wire_msg.size»;
        «IF msg.wire_msg.dynamic_size»
          /* handle dynamic string lengths */
          «FOR field : msg.wire_msg.fields.filter[type.enum_type == FieldTypeEnum.STRING]»
              /* not the last field: length is in wire message */
              __dst_msg->«field.name» = «field.name».len;
              __consumed += «field.name».len;
          «ENDFOR»
          /* last field: gets the rest of the message */
          __consumed += «msg.wire_msg.last_blob_field.name».len;
          if (__consumed > 0xffff) {
            throw std::runtime_error("Writer::«msg.name» tried to write dynamic message >= 1<<16");
          }
          __dst_msg->_len = (u16)__consumed;
        «ENDIF»

        /* start write */
        auto __allocated = __buffer.start_write(sizeof(u64) + __consumed);
        if (!__allocated) {
          return __allocated.error();
        }

        /* copy fixed part of message */
        char *__dest = (char *)*__allocated;
        assert(__dest);
        memcpy(__dest, __buf64, __len); __dest += __len;

        «IF msg.wire_msg.dynamic_size»
          «FOR field : msg.wire_msg.fields.filter[type.enum_type == FieldTypeEnum.STRING] + #[msg.wire_msg.last_blob_field]»
            /* dynamic string: «field.name» */
            memcpy(__dest, «field.name».buf, «field.name».len);
            __dest += «field.name».len;

          «ENDFOR»
        «ENDIF»
        /* finish write */
        __buffer.finish_write();

        return {};
      }

    «ENDFOR»

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

}
