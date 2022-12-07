// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*

class ProtocolGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
    fsa.generateFile(outputPath(app, "protocol.h"), generateProtocolH(app))
    fsa.generateFile(outputPath(app, "protocol.cc"), generateProtocolCc(app))
  }

  private static def generateProtocolH(App app) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include "hash.h"
    #include "transform_builder.h"

    #include <jitbuf/perfect_hash.h>
    «IF app.jit»
      #include <jitbuf/transform_builder.h>
    «ENDIF»
    #include <platform/types.h>

    #include <chrono>

    namespace «app.pkg.name»::«app.name» {

    // The Protocol class handles messages for a single connection.
    //
    class Protocol {
    public:
      using transform_t = TransformBuilder::transform_t;
      «IF app.jit»
        using TransformRecordPtr = std::shared_ptr<jitbuf::TransformRecord>;
      «ENDIF»

      // Handler function signature.
      typedef void (*handler_func_t)(void *context, u64 timestamp, char *msg_buf);

      // Type returned by handle() and handle_multiple().
      struct handle_result_t {
        int result;
        std::chrono::nanoseconds client_timestamp;
      };

      Protocol(TransformBuilder &builder);

      // Handles one message.
      //
      // Returns the client's timestamp, as well as the message length on success or an error code.
      //
      // Error codes:
      //   -ENOENT: message was not added
      //   -EACCES: message was not authenticated
      //   -EAGAIN: buffer is too small
      //
      // Note that handler function might throw.
      //
      handle_result_t handle(const char *msg, uint32_t len);

      // Handles multiple consecutive messages.
      //
      // Returns the client's timestamp, as well as the length of successfully consumed messages
      // if at least one message was processed, otherwise like handle().
      //
      handle_result_t handle_multiple(const char *msg, u64 len);

      // Adds a handler function for the given RPC ID.
      void add_handler(u16 rpc_id, void *context, handler_func_t handler_fn);

      «IF app.jit»
        // Inserts the transform for the given RPC ID.
        void insert_transform(u16 rpc_id, transform_t transform_fn, u32 size, TransformRecordPtr &transform_record);
      «ENDIF»

      // Insert an identity transform for the given RPC ID.
      void insert_identity_transform(u16 rpc_id);

      // Inserts default identity transforms for no-auth messages.
      void insert_no_auth_identity_transforms();

      // Inserts default identity transforms for need-auth messages.
      void insert_need_auth_identity_transforms();

    private:
      TransformBuilder &builder_;

      // Registered handler functions.
      struct HandlerFunc {
        void *context;
        handler_func_t handler_fn;
      };
      PerfectHash<HandlerFunc, «app.hashSize», «app.hashFunctor»> handler_funcs_;

      // Registered handler function along with transform.
      struct HandlerInfo {
        void *context;
        handler_func_t handler_fn;
        transform_t transform_fn;
        u32 size;
        «IF app.jit»
          TransformRecordPtr transform_record;
        «ENDIF»
      };
      PerfectHash<HandlerInfo, «app.hashSize», «app.hashFunctor»> handlers_;
    };

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def generateProtocolCc(App app) {
    val messages = app.messages

    /* compute an upper bound on parsed message size */
    val max_message_size =
      if (messages.size == 0)
        0
      else
        messages.map[parsed_msg.size].max

    val need_auth_msg = messages.filter[!noAuthorizationNeeded];

    '''
    «generatedCodeWarning()»

    #include "protocol.h"
    #include "transform_builder.h"
    #include "parsed_message.h"
    #include "wire_message.h"

    #include <algorithm>
    #include <iostream>
    #include <stdexcept>
    #include <string>

    namespace «app.pkg.name»::«app.name» {

    Protocol::Protocol(TransformBuilder &builder)
      : builder_(builder)
    {}

    Protocol::handle_result_t Protocol::handle(const char *msg, uint32_t len)
    {
      «IF app.spans.size == 0»
        // No spans.
        return {.result = -EINVAL, .client_timestamp = std::chrono::nanoseconds::zero()};
      «ELSE»
        // Size check: should have enough for timestamp and rpc_id.
        if (len < sizeof(u64) + sizeof(u16)) {
          /* not enough data to read headers */
          return {.result = -EAGAIN, .client_timestamp = std::chrono::nanoseconds::zero()};
        }

        // Handle timestamps.
        std::chrono::nanoseconds remote_timestamp{*(u64 const *)msg};

        msg += sizeof(u64);
        len -= sizeof(u64);

        // Get the RPC ID.
        uint16_t rpc_id = *(uint16_t *)msg;

        // Find the handler for this RPC ID.
        auto *handler = handlers_.find(rpc_id);

        if (handler == nullptr) {
          // Compile-time list of RPC IDs that need authentication.
          constexpr std::size_t need_auth_rpc_ids_count = «need_auth_msg.length»;
          constexpr u16 need_auth_rpc_ids[] = {«FOR rpc_id : need_auth_msg.map[wire_msg].map[rpc_id].sort SEPARATOR ", "»«rpc_id»«ENDFOR»};

          if (std::binary_search(need_auth_rpc_ids, need_auth_rpc_ids + need_auth_rpc_ids_count, rpc_id)) {
            // Permission denied.
            return {.result = -EACCES, .client_timestamp = remote_timestamp};
          } else {
            // Cannot find handler.
            return {.result = -ENOENT, .client_timestamp = remote_timestamp};
          }
        }

        // Safety check for message size.
        if (len < handler->size) {
          // Not enough data to read static payload.
          return {.result = -EAGAIN, .client_timestamp = remote_timestamp};
        }

        // Apply message transform.
        u64 dst_buffer[(«max_message_size» + 7) / 8]; /* 64-bit aligned dst */
        uint16_t size = handler->transform_fn(msg, (char *)dst_buffer);

        // If we didn't get all the dynamic sized part, request more bytes.
        if (size > len) {
          // Not enough data to read dynamic payload.
          return {.result = -EAGAIN, .client_timestamp = remote_timestamp};
        }

        // Call the handler function.
        handler->handler_fn(handler->context, remote_timestamp.count(), (char *)dst_buffer);

        return {.result = static_cast<int>(size + sizeof(u64)), .client_timestamp = remote_timestamp};
      «ENDIF»
    }

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
          // Error while handling the message.
          break;
        }
        assert ((u32)ret <= remaining);

        // Sanity check, should not happen.
        if (((u64)ret + processed > len) || (((u64)ret + processed) < processed)) {
          throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::handle_multiple: possible overflow");
        }

        processed += ret;
        remaining -= ret;
        ++count;
      }

      if (processed > 0) {
        return {.result = static_cast<int>(processed), .client_timestamp = client_timestamp};
      }

      // Error, return code (or in edge case of len == 0, returns 0).
      return {.result = ret, .client_timestamp = client_timestamp};
    }

    void Protocol::add_handler(u16 rpc_id, void *context, handler_func_t handler_fn)
    {
      auto *inserted = handler_funcs_.insert(rpc_id, HandlerFunc{context, handler_fn});
      if (inserted == nullptr) {
        throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::add_handler: unable to insert handler_fn for rpc_id=" + std::to_string(rpc_id));
      }
    }

    «IF app.jit»
    void Protocol::insert_transform(u16 rpc_id, transform_t transform_fn, u32 size, TransformRecordPtr &transform_record)
    {
      auto *handler_func = handler_funcs_.find(rpc_id);
      if (handler_func == nullptr) {
        throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::insert_transform: handler not found for rpc_id=" + std::to_string(rpc_id));
      }

      auto *inserted = handlers_.insert(rpc_id, HandlerInfo{
          .context = handler_func->context,
          .handler_fn = handler_func->handler_fn,
          .transform_fn = transform_fn,
          .size = size,
          .transform_record = transform_record,
      });
      if (inserted == nullptr) {
        throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::insert_transform: unable to insert transform for rpc_id=" + std::to_string(rpc_id));
      }
    }
    «ENDIF»

    void Protocol::insert_identity_transform(u16 rpc_id)
    {
      auto *handler_func = handler_funcs_.find(rpc_id);
      if (handler_func == nullptr) {
        throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::insert_identity_transform: handler not found for rpc_id=" + std::to_string(rpc_id));
      }

      auto *inserted = handlers_.insert(rpc_id, HandlerInfo{
          .context = handler_func->context,
          .handler_fn = handler_func->handler_fn,
          .transform_fn = builder_.get_identity(rpc_id),
          .size = builder_.get_identity_size(rpc_id),
          «IF app.jit»
            .transform_record = nullptr,
          «ENDIF»
      });
      if (inserted == nullptr) {
        throw std::runtime_error("«app.pkg.name»::«app.name»::Protocol::insert_identity_transform: unable to insert identity transform for rpc_id=" + std::to_string(rpc_id));
      }
    }

    void Protocol::insert_no_auth_identity_transforms()
    {
      «FOR msg : messages»
        «IF msg.noAuthorizationNeeded»
          // «msg.span.name».«msg.name»
          insert_identity_transform(«msg.wire_msg.rpc_id»);
        «ENDIF»
      «ENDFOR»
    }

    void Protocol::insert_need_auth_identity_transforms()
    {
      «FOR msg : messages»
        «IF !msg.noAuthorizationNeeded»
          // «msg.span.name».«msg.name»
          insert_identity_transform(«msg.wire_msg.rpc_id»);
        «ENDIF»
      «ENDFOR»
    }

    } // namespace «app.pkg.name»::«app.name»
    '''
  }
}
