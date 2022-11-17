// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.Span
import io.opentelemetry.render.render.Message
import io.opentelemetry.render.render.MessageType
import static io.opentelemetry.render.generator.AppGenerator.outputPath
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.SpanExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.*

class ConnectionGenerator {

  def void doGenerate(App app, IFileSystemAccess2 fsa) {
    fsa.generateFile(outputPath(app, "connection.h"), generateConnectionH(app))
    fsa.generateFile(outputPath(app, "connection.cc"), generateConnectionCc(app))
  }

  private static def generateConnectionH(App app) {
    '''
    «generatedCodeWarning()»
    #pragma once

    #include "index.h"
    #include "handles.h"
    #include "weak_refs.h"

    #include <platform/types.h>
    #include <util/fixed_hash.h>

    «FOR app_span : app.spans.filter[include !== null]»
      #include «app_span.include»
    «ENDFOR»

    namespace «app.pkg.name»::«app.name» {

    class Protocol;

    class Connection {
    public:
      Connection(Protocol &protocol, Index &index);
      ~Connection();

      void on_connection_authenticated();

      // Singleton span accessors.
      //
      «FOR span : app.spans.filter[isSingleton]»
        weak_refs::«span.name» «span.name»() const { return «span.instanceName».access(index_); }
      «ENDFOR»

      // Lookup functions for each span type.
      //
      «FOR span : app.spans.filter[conn_hash]»
        «spanLookupDeclaration(span)»
      «ENDFOR»

      // Handlers for all the incoming messages.
      //
      «FOR span : app.spans»
        «IF span.messages.length > 0»
          // span «span.name»
          «FOR msg : span.messages»
            void «app.c_name»_«msg.name»(u64 timestamp, char *msg_buf);
          «ENDFOR»
        «ENDIF»
      «ENDFOR»

      // Hashers for each span type.
      //
      «FOR span : app.spans.filter[conn_hash]»
        struct «fixedHashHasherName(span)» {
          typedef std::size_t result_type;
          result_type operator()(«span.referenceType.wireCType» const &s) const noexcept;
        };
      «ENDFOR»

      // Hash table types for each span type.
      //
      «FOR span : app.spans.filter[conn_hash]»
        using «fixedHashTypeName(span)» = FixedHash<«span.referenceType.wireCType», handles::«span.name», «span.pool_size», «fixedHashHasherName(span)»>;
      «ENDFOR»

      // Pools for each span type.
      //
      «FOR span : app.spans.filter[conn_hash]»
        «fixedHashTypeName(span)» «fixedHashName(span)»;
      «ENDFOR»

      struct MessageStatistics {
        MessageStatistics();

        // For each message, calls `f(module_name, message_name, severity, count)`
        void foreach(std::function<void(std::string_view, std::string_view, int, u64)> const& f);

        // Message counts.
        struct {
          «FOR span : app.spans»
            «FOR msg : span.messages»
              u64 «app.c_name»_«msg.name»;
            «ENDFOR»
          «ENDFOR»
        } counts;

        // Message count change indications.
        struct {
          «FOR span : app.spans»
            «FOR msg : span.messages»
              unsigned int «app.c_name»_«msg.name» : 1;
            «ENDFOR»
          «ENDFOR»
        } changed;

      } message_stats;

      struct MessageErrors {
        MessageErrors();

        // For each message, calls `f(module_name, message_name, error_name, count)`
        void foreach(std::function<void(std::string_view, std::string_view, std::string_view, u64)> const &f);

        // Error counts.
        struct {
          «FOR span : app.spans»
            «FOR msg : span.messages»
              «FOR error_name : msg.errors»
                u64 «app.c_name»_«msg.name»_«error_name»;
              «ENDFOR»
            «ENDFOR»
          «ENDFOR»
        } counts;

        // Error count change indications.
        struct {
          «FOR span : app.spans»
            «FOR msg : span.messages»
              «FOR error_name : msg.errors»
                unsigned int «app.c_name»_«msg.name»_«error_name» : 1;
              «ENDFOR»
            «ENDFOR»
          «ENDFOR»
        } changed;

      } message_errors;

      Index &index() { return index_; }

    private:
      Protocol &protocol_;
      Index &index_;

      // Singleton spans maintain one instance per span.
      //
      «FOR span : app.spans.filter[isSingleton]»
        handles::«span.name» «span.instanceName»;
      «ENDFOR»
    };

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def generateConnectionCc(App app) {
    '''
    «generatedCodeWarning()»

    #include "connection.h"
    #include "protocol.h"
    #include "parsed_message.h"
    #include "weak_refs.inl"
    #include "containers.inl"

    #include <util/lookup3.h>
    #include <util/render.h>

    #include <algorithm>
    #include <stdexcept>

    namespace «app.pkg.name»::«app.name» {

    Connection::Connection(Protocol &protocol, Index &index)
      : protocol_(protocol), index_(index)
    {
      «FOR span : app.spans.filter[isSingleton] SEPARATOR "\n"»
        if (auto ref = index_.«span.name».alloc(); ref.valid()) {
          «span.instanceName» = ref.to_handle();
        } else {
          throw std::runtime_error("«app.pkg.name»::«app.name»::Connection - could not allocate «span.name» handle");
        }
      «ENDFOR»

      «FOR span : app.spans»
        «FOR msg : span.messages»
          «IF msg.noAuthorizationNeeded»
            protocol_.add_handler(«msg.wire_msg.rpc_id», this, &dispatch_protocol_member_handler<Connection, &Connection::«app.c_name»_«msg.name»>);
          «ENDIF»
        «ENDFOR»
      «ENDFOR»

      // Only identity transformations are currently supported.
      protocol_.insert_no_auth_identity_transforms();
    }

    Connection::~Connection()
    {
      // Clean up singleton spans.
      //
      «FOR span : app.spans.filter[isSingleton]»
        «span.instanceName».put(index_);
      «ENDFOR»

      //
      // Clean up span pools.
      //

      «FOR span : app.spans.filter[conn_hash] SEPARATOR "\n"»
        for (auto handle_loc : «fixedHashName(span)».allocated()) {
          «IF span.impl !== null»
            «FOR msg : span.messages.filter[type == MessageType.END && reference_field.name == "_ref"]»
            {
              struct «msg.parsed_msg.struct_name» msg = {
                ._rpc_id = «msg.wire_msg.rpc_id»,
                ._ref = handle_loc,
              };
              auto span_ref = «fixedHashName(span)»[handle_loc].access(index_);
              span_ref.impl().«msg.name»(span_ref, 0, &msg);
            }
            «ENDFOR»
          «ENDIF»
          «fixedHashName(span)»[handle_loc].put(index_);
        }
      «ENDFOR»
    }

    void Connection::on_connection_authenticated()
    {
      «FOR span : app.spans»
        «FOR msg : span.messages»
          «IF !msg.noAuthorizationNeeded»
            protocol_.add_handler(«msg.wire_msg.rpc_id», this, &dispatch_protocol_member_handler<Connection, &Connection::«app.c_name»_«msg.name»>);
          «ENDIF»
        «ENDFOR»
      «ENDFOR»

      // Only identity transformations are currently supported.
      protocol_.insert_need_auth_identity_transforms();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Span accessor function implementations.
    //

    «FOR span : app.spans.filter[conn_hash] SEPARATOR "\n"»
      «spanLookupImplementation(span)»
    «ENDFOR»

    ////////////////////////////////////////////////////////////////////////////////
    // Handler function implementations.
    //

    «FOR span : app.spans»
      «FOR msg : span.messages SEPARATOR "\n"»
        «handlerImplementation(span, msg, app)»
      «ENDFOR»
    «ENDFOR»

    ////////////////////////////////////////////////////////////////////////////////
    // Hasher implementations for spans.
    //

    «FOR span : app.spans.filter[conn_hash] SEPARATOR "\n"»
      typename Connection::«fixedHashHasherName(span)»::result_type
      Connection::«fixedHashHasherName(span)»::operator()(«span.referenceType.wireCType» const &s) const noexcept
      {
        result_type val = 0xBFFB7A00;
        «FOR field : #[span.messages.head.reference_field]»
          «IF field.isArray && field.type.isShortString»
          «ELSEIF field.type.isShortString»
          «ELSEIF RenderGenerator::integerTypeSize(field.type.enum_type) % 4 == 0»
            // «field.name» is a primitive type, is multiple of 4 bytes: will hash in 4-byte words.
            val = (result_type)lookup3_hashword((u32 *)&s, «RenderGenerator::fieldSize(field) / 4», val + «RenderGenerator::fieldSize(field)»);
          «ELSE»
            // «field.name» is a plain variable: will hash individual bytes.
            val = (result_type)lookup3_hashlittle((char *)&s, «RenderGenerator::fieldSize(field)», val + «RenderGenerator::fieldSize(field)»);
          «ENDIF»
        «ENDFOR»
        return val;
      }
    «ENDFOR»

    ////////////////////////////////////////////////////////////////////////////////
    // Message statistics.
    //

    Connection::MessageStatistics::MessageStatistics()
    {
      «FOR span : app.spans»
        «FOR msg : span.messages»
          counts.«app.c_name»_«msg.name» = 0;
        «ENDFOR»
      «ENDFOR»

      memset(&changed, 0, sizeof(changed));
    }

    void Connection::MessageStatistics::foreach(std::function<void(std::string_view, std::string_view, int, u64)> const& f)
    {
      «FOR span : app.spans»
        «FOR msg : span.messages»
          if (changed.«app.c_name»_«msg.name») {
            f("«app.c_name»", "«msg.name»", «msg.severity», counts.«app.c_name»_«msg.name»);
          }
        «ENDFOR»
      «ENDFOR»

      memset(&changed, 0, sizeof(changed));
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Message error counts.
    //

    Connection::MessageErrors::MessageErrors()
    {
      memset(&counts, 0, sizeof(counts));
      memset(&changed, 0, sizeof(changed));
    }

    void Connection::MessageErrors::foreach(std::function<void(std::string_view, std::string_view, std::string_view, u64)> const &f)
    {
      «FOR span : app.spans»
        «FOR msg : span.messages»
          «FOR error_name : msg.errors»
            if (changed.«app.c_name»_«msg.name»_«error_name») {
              f("«app.c_name»", "«msg.name»", "«error_name»", counts.«app.c_name»_«msg.name»_«error_name»);
            }
          «ENDFOR»
        «ENDFOR»
      «ENDFOR»

      memset(&changed, 0, sizeof(changed));
    }

    } // namespace «app.pkg.name»::«app.name»
    '''
  }

  private static def spanLookupDeclaration(Span span) {
    '''
    weak_refs::«span.name» get_«span.name»(«span.referenceType.wireCType» key);
    '''
  }

  private static def spanLookupImplementation(Span span) {
    '''
    weak_refs::«span.name» Connection::get_«span.name»(«span.referenceType.wireCType» key)
    {
      if (auto pos = «fixedHashName(span)».find(key); pos.index != «fixedHashName(span)».invalid) {
        return pos.entry->access(index_);
      } else {
        return {index_, weak_refs::«span.name»::invalid};
      }
    }
    '''
  }

  private static def handlerImplementation(Span span, Message msg, App app) {
    val pmsg = msg.parsed_msg

    '''
    void Connection::«app.c_name»_«msg.name»(u64 t, char *msg_buf)
    {
      [[maybe_unused]] struct «pmsg.struct_name» *msg = (struct «pmsg.struct_name» *)msg_buf;

      «IF msg.type == MessageType.START»
        // "start" message: allocate/lookup and insert into the pool.
        «IF span.index === null»
          auto ref = index_.«span.name».alloc();
        «ELSE»
          keys::«span.name» key;
          «FOR field : span.index.keys»
            «IF (field instanceof Field) && (field as Field).isIsArray»
              static_assert(sizeof(key.«field.name») == sizeof(msg->«field.name»));
              std::copy_n(std::begin(msg->«field.name»), «(field as Field).array_size», std::begin(key.«field.name»));
            «ELSE»
              key.«field.name» = msg->«field.name»;
            «ENDIF»
          «ENDFOR»
          auto ref = index_.«span.name».by_key(key);
        «ENDIF»
        if (!ref.valid()) {
          // Could not allocate span.
          message_errors.counts.«app.c_name»_«msg.name»_span_alloc_failed += 1;
          message_errors.changed.«app.c_name»_«msg.name»_span_alloc_failed = 1;
          return;
        }

        auto pos = «fixedHashName(span)».insert(msg->«msg.reference_field.name», std::move(ref));
        if (pos.index == «fixedHashName(span)».invalid) {
          if («fixedHashName(span)».full()) {
            // Span handle pool full.
            message_errors.counts.«app.c_name»_«msg.name»_span_pool_full += 1;
            message_errors.changed.«app.c_name»_«msg.name»_span_pool_full = 1;
          } else {
            auto find_pos = «fixedHashName(span)».find(msg->«msg.reference_field.name»);
            if (find_pos.index != «fixedHashName(span)».invalid) {
              // Already exists.
              message_errors.counts.«app.c_name»_«msg.name»_duplicate_ref += 1;
              message_errors.changed.«app.c_name»_«msg.name»_duplicate_ref = 1;
            } else {
              // Pool insert failed.
              message_errors.counts.«app.c_name»_«msg.name»_span_insert_failed += 1;
              message_errors.changed.«app.c_name»_«msg.name»_span_insert_failed = 1;
            }
          }
          // Ignoring message -- failed to allocate or lookup span.
          return;
        }
      «ELSEIF (!span.isSingleton)»
        // Get the destination span.
        auto pos = «fixedHashName(span)».find(msg->«msg.reference_field.name»);
        if (pos.index == «fixedHashName(span)».invalid) {
          // Find failed.
          message_errors.counts.«app.c_name»_«msg.name»_span_find_failed += 1;
          message_errors.changed.«app.c_name»_«msg.name»_span_find_failed = 1;
          return;
        }
      «ENDIF»

      «IF span.impl !== null»
      {
        // Call the span's handler.
        «IF span.isSingleton»
        auto entry = &«span.instanceName»;
        «ELSE»
        auto entry = pos.entry;
        «ENDIF»
        auto span_ref = entry->access(index_);
        span_ref.impl().«msg.name»(span_ref, t, msg);
      }
      «ENDIF»

      «IF msg.type == MessageType.END»
        // "end" message: remove from pool.
        «fixedHashName(span)»[pos.index].put(index_);
        bool erased = «fixedHashName(span)».erase(msg->«msg.reference_field.name»);
        if (erased != true) {
          // Erase failed.
          message_errors.counts.«app.c_name»_«msg.name»_span_erase_failed += 1;
          message_errors.changed.«app.c_name»_«msg.name»_span_erase_failed = 1;
        }
      «ENDIF»

      // Update message statistics.
      //
      message_stats.counts.«app.c_name»_«msg.name» += 1;
      message_stats.changed.«app.c_name»_«msg.name» = 1;
    }
    '''
  }

}
