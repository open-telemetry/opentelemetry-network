// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0


package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.FieldType;
import io.opentelemetry.render.render.XPackedMessage;

public class XPackedMessageExtensions {
  public static String cType(final XPackedMessage packedMessage, final FieldType fieldType) {
    return FieldTypeExtensions.cType(fieldType, packedMessage.isPackedStrings());
  }
}