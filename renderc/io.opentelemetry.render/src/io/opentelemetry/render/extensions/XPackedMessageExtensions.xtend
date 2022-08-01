// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import io.opentelemetry.render.render.XPackedMessage
import io.opentelemetry.render.render.FieldType
import static extension io.opentelemetry.render.extensions.FieldTypeExtensions.cType

class XPackedMessageExtensions {
	static def cType(XPackedMessage packed_msg, FieldType fieldType)
	{
		fieldType.cType(packed_msg.packedStrings)
	}

}
