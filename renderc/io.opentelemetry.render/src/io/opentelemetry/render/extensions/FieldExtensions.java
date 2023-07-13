// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.Field;
import io.opentelemetry.render.render.FieldType;

public class FieldExtensions {
  public static int size(final Field field, final boolean packedStrings) {
    int numOfElements = field.isIsArray() ? field.getArray_size() : 1;
    int size = FieldTypeExtensions.size(field.getType(), packedStrings);
    return (size * numOfElements);
  }

  public static CharSequence arraySuffix(final Field field) {
    if (field.isIsArray()) {
      return "[" + field.getArray_size() + "]";
    }
      return "";
}

  public static CharSequence cType(final Field field) {
    FieldType type = field.getType();
    int expression = field.isIsArray() ? field.getArray_size() : -1;
    return FieldTypeExtensions.cType(type, expression);
  }
}
