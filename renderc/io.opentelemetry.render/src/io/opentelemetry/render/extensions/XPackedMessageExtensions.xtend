// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.extensions

import net.flowmill.render.render.XPackedMessage
import net.flowmill.render.render.FieldType
import static extension net.flowmill.render.extensions.FieldTypeExtensions.cType

class XPackedMessageExtensions {
	static def cType(XPackedMessage packed_msg, FieldType fieldType)
	{
		fieldType.cType(packed_msg.packedStrings)
	}

}
