// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.Field;
import io.opentelemetry.render.render.FieldType;


public class FieldExtensions {
  
  public static int size(final Field field, final boolean packedStrings) {
    int xIfExpression;
    boolean isArray = field.isIsArray();
    if (isArray) {
      xIfExpression = field.getArray_size();
    } else {
      xIfExpression = 1;
    }
    final int numOfElements = xIfExpression;
    int size = FieldTypeExtensions.size(field.getType(), packedStrings);
    return (size * numOfElements);
  }

 
  public static CharSequence arraySuffix(final Field field) {
    CharSequence xIfExpression = null;
    boolean isArray = field.isIsArray();
    if (isArray) {
      StringBuilder builder = new StringBuilder();
      builder.append("[");
      int arraySize = field.getArray_size();
      builder.append(arraySize);
      builder.append("]");
      xIfExpression = builder;
    } else {
      xIfExpression = "";
    }
    return xIfExpression;
  }

  public static CharSequence cType(final Field field) {
    FieldType type = field.getType();
    int xIfExpression;
    boolean isArray = field.isIsArray();
    if (isArray) {
      xIfExpression = field.getArray_size();
    } else {
      xIfExpression = (-1);
    }
    return FieldTypeExtensions.cType(type, xIfExpression);
  }
}


