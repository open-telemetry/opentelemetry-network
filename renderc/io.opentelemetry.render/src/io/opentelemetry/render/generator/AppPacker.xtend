// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import java.util.LinkedList
import java.util.Vector
import io.opentelemetry.render.render.Document
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.FieldTypeEnum
import io.opentelemetry.render.render.Message
import io.opentelemetry.render.render.MessageType
import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.XPackedMessage
import io.opentelemetry.render.render.impl.RenderFactoryImpl
import org.eclipse.xtend.lib.annotations.Accessors
import org.eclipse.emf.ecore.util.EcoreUtil
import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.*

/**
 * Represents a message together with field placement
 */
@Accessors(PUBLIC_GETTER, PROTECTED_SETTER)
class AppPacker {

  // RPC ID needs to fit in a u16.
  // Upper half of the u16 range is reserved for special messages and future use.
  public static val rpcIdRangeMin = 0
  public static val rpcIdRangeMax = 32767

  public static val pulseMessageRpcId = 65535
  public static val pulseMessageName = "pulse"

  static def populate(App app) {
    val factory = new RenderFactoryImpl

    app.spans.add(makePulseSpan(factory))

    for (msg : app.messages) {
      makeMessage(factory, msg, true)
      makeMessage(factory, msg, false)
    }

    annotateClass(app)

    return app
  }

  private static def makePulseSpan(RenderFactoryImpl factory) {
    val span = factory.createSpan()

    span.name = "__pulsar"
    span.isSingleton = true

    val pulseMessage = factory.createMessage()

    pulseMessage.id = pulseMessageRpcId
    pulseMessage.name = pulseMessageName
    pulseMessage.type = MessageType.LOG
    pulseMessage.pipelineOnly = true

    span.messages.add(pulseMessage)

    return span
  }

  private static def createField(RenderFactoryImpl factory, int id, String name, FieldTypeEnum type) {
    var field = factory.createField()

    field.id = id
    field.name = name
    field.type = factory.createFieldType()
    field.type.enum_type = type
    field.isArray = false

    return field
  }

  // Clones the given Field to a new Field object with the given ID
  private static def copyField(Field field) {
    val result = EcoreUtil.copy(field)
    return result
  }

  private static def makeMessage(RenderFactoryImpl factory, Message msg, boolean packedStrings) {
    var packedMsg = factory.createXPackedMessage()

    if (msg.type == MessageType.MSG) {
      // special start and end messages
      if (msg.name == "_start") {
        msg.name = msg.span.name + "_start"
        msg.type = MessageType.START;

        if (msg.span.index !== null) {
          var field_ids = msg.fields.map[id]
          var field_id = if (field_ids.empty) 1 else (field_ids.max + 1)
          for (key_field : msg.span.index.keys.filter(Field)) {
            var field = copyField(key_field)
            field.id = field_id++
            msg.fields.add(field)
          }
        }
      } else if (msg.name == "_end") {
        msg.name = msg.span.name + "_end"
        msg.type = MessageType.END;
      }

      // add reference field if not already there
      if (msg.referenceEmbedded == false) {
        var field = createField(factory, 0, "_ref", FieldTypeEnum.U64);
        msg.fields.add(field)
        msg.reference_field = field
        msg.referenceEmbedded = true
      }
    }

    if (packedStrings) {
      msg.wire_msg = packedMsg
    } else {
      msg.parsed_msg = packedMsg
    }

    packedMsg.packedStrings = packedStrings

    packMessageFields(packedMsg, packedStrings)
    annotateMessage(packedMsg, packedStrings)
    makeMessageDescriptors(packedMsg, packedStrings)

    return packedMsg
  }

  /**
   * Packs the given Render message
   * @param msg: the message to pack
   * @param packedStrings: true if strings and their lengths should be packed
   *   tightly with total length encoded at the beginning, and strings at the
   *   end of the message; false to add char * and lengths for every string
   */
  private static def packMessageFields(XPackedMessage packedMsg, boolean packedStrings) {
    val msg = packedMsg.eContainer as Message
    val fields = new LinkedList(msg.fields)

    /* rpc ID is at pos 0, start at pos 2 */
    var pos = 2

    if (packedStrings && msg.fields.exists[type.enum_type == FieldTypeEnum.STRING]) {
      /**
       * if there are blobs, we have a 'total_length' field after the
       * rpc_id and we do not encode the last blob's length (it is what
       * remains after subtracting the other fields and blobs)
       */

      /* leave space for the length field */
      pos += 2

      /* remove the last blob, so it doesn't get its own length field */
      fields.remove(fields.findLast[type.enum_type == FieldTypeEnum.STRING])
    }

    /* group fields by their alignment */
    val by_alignment = fields.groupBy[type.alignment(packedStrings)]

    // how many fields do we have to position?
    var remaining = by_alignment.values().map[size].reduce[p1, p2| p1 + p2] ?: 0

    while (remaining > 0) {
      val start_pos = pos

      // try the largest value that is already aligned
      val exact_alignment =
        by_alignment
          // should be exactly aligned
          .filter[align , flds | start_pos.bitwiseAnd(align - 1) == 0]
          // fields should not be empty
          .filter[align, flds | !flds.empty]

      val align =
        if (!exact_alignment.empty) {
          // get the largest exact alignment
          exact_alignment.keySet.max
        } else {
          // no alignment is right, just get the smallest alignment
          by_alignment.filter[align, flds | !flds.empty].keySet.min
        }

      val field = by_alignment.get(align).remove(0)
      val aligned_pos = (pos + align - 1).bitwiseAnd((align - 1).bitwiseNot)

      packedMsg.fields.add(field)

      if (packedStrings) {
        field.wire_pos = aligned_pos
      } else {
        field.parsed_pos = aligned_pos
      }

      pos = aligned_pos + field.size(packedStrings)

      remaining--
    }

    // pos now points after the last serialized byte, which is the size
    packedMsg.size = pos
  }

  private static def annotateMessage(XPackedMessage packedMsg, boolean packedStrings) {
    val msg = packedMsg.eContainer as Message
    val app = msg.eContainer.eContainer as App
    val document = app.eContainer as Document

    // find rpc_id
    var rpcIdFound = false
    if (msg.id > rpcIdRangeMax) {
      // messages with IDs outside normal RPC ID range are special messages
      packedMsg.rpc_id = msg.id
      rpcIdFound = true
    } else {
      val namespace = document.namespace
      val appMap = namespace.mappings.findFirst[x | x.app == app ]
      var rpcIndex = msg.id
      for (range : appMap.ranges) {
        val rangeEnd = if (range.hasEnd) range.end + 1 else range.start + 1
        val rangeSize = rangeEnd - range.start
        if (!rpcIdFound && (rpcIndex < rangeSize)) {
          packedMsg.rpc_id = range.start + rpcIndex
          rpcIdFound = true
        }
        rpcIndex -= rangeSize
      }
    }

    if (!rpcIdFound) {
      throw new RuntimeException("could not find rpc_id for message " + msg)
    }

    val c_package = app.name.split("\\.").join("_")

    packedMsg.attr_name = if (packedStrings) "jb" else "jsrv"
    packedMsg.struct_name = packedMsg.attr_name + '_' + c_package + '__' + msg.name
    packedMsg.descriptor_name = c_package + '__' + msg.name + (if (packedStrings) '_descriptor' else '__ext_descriptor')

    if (msg.fields.exists[type.enum_type == FieldTypeEnum.STRING]) {
      packedMsg.dynamic_size = true
      packedMsg.last_blob_field = msg.fields.findLast[type.enum_type == FieldTypeEnum.STRING]
      packedMsg.last_blob_field.wire_pos = 0x7fffffff
    } else {
      packedMsg.dynamic_size = false
    }
  }

  private static def annotateClass(App app) {
    val basename = (app.pkg.name + '/' + app.name + '/').replace('.','/')

    app.c_name = app.name.replace('.','_')
    app.jb_h = basename + "wire_message.h"
    app.jsrv_h = basename + "parsed_message.h"
    app.descriptor_h = basename + "descriptor.h"
    app.bpf_h = basename + "bpf.h"
  }

  private static def makeMessageDescriptors(XPackedMessage packedMsg, boolean packedStrings) {
    val flags = 0
    val rpcId = packedMsg.rpc_id

    /* get all fields, including the last blob field */
    val fields = new Vector<Field>(packedMsg.fields)
    if (packedStrings && packedMsg.dynamic_size) {
      fields.add(packedMsg.last_blob_field)
    }

    val fields_u16 = new Vector<Integer>
    val arrays_u16 = new Vector<Integer>

    for (field : fields) {
      val int isArr = if (field.isArray) 1 else 0

      val ftype = switch(field.type.enum_type) {
        case FieldTypeEnum.U8:  0
        case FieldTypeEnum.U16:  1
        case FieldTypeEnum.U32:  2
        case FieldTypeEnum.U64:  3
        case FieldTypeEnum.U128:  5
        case FieldTypeEnum.S8:  0
        case FieldTypeEnum.S16:  1
        case FieldTypeEnum.S32:  2
        case FieldTypeEnum.S64:  3
        case FieldTypeEnum.S128:  5
        case FieldTypeEnum.STRING: 4
      }

      fields_u16.add((isArr << 15) + (ftype << 12) + field.id)

      if (field.isIsArray) {
        arrays_u16.add(field.array_size)
      }
    }

    val descriptor = new Vector<Integer>
    descriptor.add(flags)
    descriptor.add(rpcId)
    descriptor.add(fields_u16.size)
    descriptor.add(arrays_u16.size)
    descriptor.addAll(fields_u16)
    descriptor.addAll(arrays_u16)

    packedMsg.descriptor.addAll(descriptor)
  }
}
