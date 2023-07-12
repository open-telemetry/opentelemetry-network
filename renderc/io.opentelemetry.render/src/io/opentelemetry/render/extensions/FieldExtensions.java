// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.Field;
import io.opentelemetry.render.render.FieldType;

public class FieldExtensions {

  public static int size(final Field field, final boolean packedStrings) {
    int expression;
    if (field.isIsArray()) {
      expression = field.getArray_size();
    } else {
      expression = 1;
    }
    final int numOfElements = expression;
    int size = FieldTypeExtensions.size(field.getType(), packedStrings);
    return (size * numOfElements);
  }

  public static CharSequence arraySuffix(final Field field) {
    CharSequence expression = null;
    if (field.isIsArray()) {
      StringBuilder builder = new StringBuilder();
      builder.append("[");
      int arraySize = field.getArray_size();
      builder.append(arraySize);
      builder.append("]");
      expression = builder;
    } else {
      expression = "";
    }
    return expression;
  }

  public static CharSequence cType(final Field field) {
    FieldType type = field.getType();
    int expression;
    if (field.isIsArray()) {
      expression = field.getArray_size();
    } else {
      expression = (-1);
    }
    return FieldTypeExtensions.cType(type, expression);
  }
}