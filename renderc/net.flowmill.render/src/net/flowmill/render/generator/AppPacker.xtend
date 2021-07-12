//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

package net.flowmill.render.generator

import java.util.LinkedList
import java.util.Vector
import net.flowmill.render.render.Document
import net.flowmill.render.render.Field
import net.flowmill.render.render.FieldTypeEnum
import net.flowmill.render.render.Message
import net.flowmill.render.render.MessageType
import net.flowmill.render.render.App
import net.flowmill.render.render.XPackedMessage
import net.flowmill.render.render.impl.RenderFactoryImpl
import org.eclipse.xtend.lib.annotations.Accessors
import org.eclipse.emf.ecore.util.EcoreUtil
import static extension net.flowmill.render.extensions.MessageExtensions.*
import static extension net.flowmill.render.extensions.FieldExtensions.*
import static extension net.flowmill.render.extensions.FieldTypeExtensions.*

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

		for (span : app.spans) {
			for (msg : span.messages) {
				make_message(factory, msg, true)
				make_message(factory, msg, false)
			}
		}

		annotate_class(app)

		return app
	}

	static private def makePulseSpan(RenderFactoryImpl factory)
	{
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

	static private def createField(RenderFactoryImpl factory, int id, String name, FieldTypeEnum type) {
		var field = factory.createField()

		field.id = id
		field.name = name
		field.type = factory.createFieldType()
		field.type.enum_type = type
		field.isArray = false

		return field
	}

	// Clones the given Field to a new Field object with the given ID
	static private def copyField(Field field) {
		val result = EcoreUtil.copy(field)
		return result
	}

	static def make_message(RenderFactoryImpl factory, Message msg, boolean packedStrings)
	{
		var packed_msg = factory.createXPackedMessage()

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

		if (packedStrings)
			msg.wire_msg = packed_msg
		else
			msg.parsed_msg = packed_msg

		packed_msg.packedStrings = packedStrings

		pack_message_fields(packed_msg, packedStrings)
		annotate_message(packed_msg, packedStrings)
		make_message_descriptors(packed_msg, packedStrings)

		return packed_msg
	}

	/**
	 * Packs the given Render message
	 * @param msg: the message to pack
	 * @param packedStrings: true if strings and their lengths should be packed
	 *   tightly with total length encoded at the beginning, and strings at the
	 *   end of the message; false to add char * and lengths for every string
	 */
	static def pack_message_fields(XPackedMessage packed_msg,
			boolean packedStrings)
	{
		val msg = packed_msg.eContainer as Message
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
	        	if (!exact_alignment.empty)
	        		// get the largest exact alignment
					exact_alignment.keySet.max
				else
		        	// no alignment is right, just get the smallest alignment
					by_alignment.filter[align, flds | !flds.empty].keySet.min

			val field = by_alignment.get(align).remove(0)
		    val aligned_pos = (pos + align - 1).bitwiseAnd((align - 1).bitwiseNot)
		    packed_msg.fields.add(field)
		    if (packedStrings)
		    	field.wire_pos = aligned_pos
		    else
		    	field.parsed_pos = aligned_pos
		    pos = aligned_pos + field.size(packedStrings)
			remaining--
		}

		// pos now points after the last serialized byte, which is the size
		packed_msg.size = pos
	}

	static def annotate_message(XPackedMessage packed_msg,
		boolean packedStrings)
	{
		val msg = packed_msg.eContainer as Message
		val app = msg.eContainer.eContainer as App
		val document = app.eContainer as Document

		// find rpc_id
		var rpc_id_found = false
		if (msg.id > rpcIdRangeMax) {
			// messages with IDs outside normal RPC ID range are special messages
			packed_msg.rpc_id = msg.id
			rpc_id_found = true
		} else {
			val namespace = document.namespace
			val app_map = namespace.mappings.findFirst[x | x.app == app ]
			var rpc_index = msg.id
			for (range : app_map.ranges) {
				val range_end = if (range.hasEnd) range.end + 1 else range.start + 1
				val range_size = range_end - range.start
				if (!rpc_id_found && (rpc_index < range_size)) {
					packed_msg.rpc_id = range.start + rpc_index
					rpc_id_found = true
				}
				rpc_index -= range_size
			}
		}

		if (!rpc_id_found) {
			throw new RuntimeException("could not find rpc_id for message " + msg)
		}

		val c_package = app.name.split("\\.").join("_")

		packed_msg.attr_name = if (packedStrings) "jb" else "jsrv"
	    packed_msg.struct_name = packed_msg.attr_name + '_' + c_package + '__' + msg.name
	    packed_msg.descriptor_name = c_package + '__' + msg.name +
	    	(if (packedStrings) '_descriptor' else '__ext_descriptor')

	    if (msg.fields.exists[type.enum_type == FieldTypeEnum.STRING]) {
	    	packed_msg.dynamic_size = true
	    	packed_msg.last_blob_field = msg.fields.findLast[type.enum_type == FieldTypeEnum.STRING]
	    	packed_msg.last_blob_field.wire_pos = 0x7fffffff
	    } else {
	    	packed_msg.dynamic_size = false
	    }
	}

	static def annotate_class(App app)
	{
		val document = app.eContainer as Document
		val app_name =  document.package.name

	    val basename = (app_name + '/' + app.name).replace('.','/')

	    app.c_name = app.name.replace('.','_')
	    app.jb_h = basename + '.wire_message.h'
	    app.jsrv_h = basename + '.parsed_message.h'
	    app.descriptor_h = basename + ".descriptor.h"
	    app.bpf_h = basename + '.bpf.h'
	}

	static def make_message_descriptors(XPackedMessage packed_msg,
		boolean packedStrings
	) {
		val flags = 0
		val rpc_id = packed_msg.rpc_id

		/* get all fields, including the last blob field */
		val fields = new Vector<Field>(packed_msg.fields)
		if (packedStrings && packed_msg.dynamic_size)
			fields.add(packed_msg.last_blob_field)

		val fields_u16 = new Vector<Integer>
		val arrays_u16 = new Vector<Integer>

        for (field : fields) {

        	val int is_arr = if (field.isArray) 1 else 0;
        	val ftype = switch(field.type.enum_type) {
				case FieldTypeEnum.U8:	0
				case FieldTypeEnum.U16:	1
				case FieldTypeEnum.U32:	2
				case FieldTypeEnum.U64:	3
				case FieldTypeEnum.U128:	5
				case FieldTypeEnum.S8:	0
				case FieldTypeEnum.S16:	1
				case FieldTypeEnum.S32:	2
				case FieldTypeEnum.S64:	3
				case FieldTypeEnum.S128:	5
				case FieldTypeEnum.STRING: 4
			}

            fields_u16.add((is_arr << 15) + (ftype << 12) + field.id)
            if (field.isIsArray)
                arrays_u16.add(field.array_size)

        }

        val descriptor = new Vector<Integer>
        descriptor.add(flags)
        descriptor.add(rpc_id)
        descriptor.add(fields_u16.size)
        descriptor.add(arrays_u16.size)
        descriptor.addAll(fields_u16)
        descriptor.addAll(arrays_u16)

        packed_msg.descriptor.addAll(descriptor)
	}
}
