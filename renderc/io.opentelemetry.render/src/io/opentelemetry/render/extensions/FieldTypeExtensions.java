// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.FieldType;
import io.opentelemetry.render.render.FieldTypeEnum;

public class FieldTypeExtensions {
  public static String cType(final FieldType fieldType, final boolean packedStrings) {
    String switchResult = null;
    FieldTypeEnum enumType = fieldType.getEnum_type();
    if (enumType != null) {
      switch (enumType) {
        case U8:
          switchResult = "uint8_t";
          break;
        case U16:
          switchResult = "uint16_t";
          break;
        case U32:
          switchResult = "uint32_t";
          break;
        case U64:
          switchResult = "uint64_t";
          break;
        case U128:
          switchResult = "unsigned __int128";
          break;
        case S8:
          switchResult = "int8_t";
          break;
        case S16:
          switchResult = "int16_t";
          break;
        case S32:
          switchResult = "int32_t";
          break;
        case S64:
          switchResult = "int64_t";
          break;
        case S128:
          switchResult = "__int128";
          break;
        case STRING:
          switchResult = packedStrings ? "uint16_t" : "struct jb_blob";
          break;
        default:
          break;
      }
    }
    return switchResult;
  }

  public static CharSequence cType(final FieldType type, final int arraySize) {
    String expression =
        type.isIsShortString()
            ? "short_string<" + type.getSize() + ">"
            : type.getEnum_type().getLiteral();
    final String nonArrayType = expression;
    CharSequence blockExpression =
        arraySize >= 0 ? "std::array<" + nonArrayType + "," + arraySize + ">" : nonArrayType;
    return blockExpression;
  }

  public static String wireCType(final FieldType fieldType) {
    return FieldTypeExtensions.cType(fieldType, true);
  }

  public static String parsedCType(final FieldType fieldType) {
    return FieldTypeExtensions.cType(fieldType, false);
  }

  public static int size(final FieldType fieldType, final boolean packedStrings) {
    int expression;
    if (fieldType.isIsShortString()) {
      expression = fieldType.getSize();
    } else {
      int switchResult = (int) 0;
      FieldTypeEnum enumType = fieldType.getEnum_type();
      if (enumType != null) {
        switch (enumType) {
          case U8:
            switchResult = 1;
            break;
          case U16:
            switchResult = 2;
            break;
          case U32:
            switchResult = 4;
            break;
          case U64:
            switchResult = 8;
            break;
          case U128:
            switchResult = 16;
            break;
          case S8:
            switchResult = 1;
            break;
          case S16:
            switchResult = 2;
            break;
          case S32:
            switchResult = 4;
            break;
          case S64:
            switchResult = 8;
            break;
          case S128:
            switchResult = 16;
            break;
          case STRING:
            switchResult = packedStrings ? 2 : 8;
            break;
          default:
            break;
        }
      }
      expression = switchResult;
    }
    return expression;
  }

  public static int wireSize(final FieldType fieldType) {
    return FieldTypeExtensions.size(fieldType, true);
  }

  public static int parsedSize(final FieldType fieldType) {
    return FieldTypeExtensions.size(fieldType, false);
  }

  public static int alignment(final FieldType fieldType, final boolean packedStrings) {
    int expression;
    if (fieldType.isIsShortString()) {
      expression = 1;
    } else {
      int switchResult = (int) 0;
      FieldTypeEnum enumType = fieldType.getEnum_type();
      if (enumType != null) {
        switch (enumType) {
          case U8:
            switchResult = 1;
            break;
          case U16:
            switchResult = 2;
            break;
          case U32:
            switchResult = 4;
            break;
          case U64:
            switchResult = 8;
            break;
          case U128:
            switchResult = 16;
            break;
          case S8:
            switchResult = 1;
            break;
          case S16:
            switchResult = 2;
            break;
          case S32:
            switchResult = 4;
            break;
          case S64:
            switchResult = 8;
            break;
          case S128:
            switchResult = 16;
            break;
          case STRING:
            switchResult = packedStrings ? 2 : 8;
            break;
        }
      }
      expression = switchResult;
    }
    return expression;
  }

  public static int wireAlignment(final FieldType fieldType) {
    return FieldTypeExtensions.alignment(fieldType, true);
  }

  public static int parsedAlignment(final FieldType fieldType) {
    return FieldTypeExtensions.alignment(fieldType, false);
  }

  public static boolean isInt(final FieldType fieldType) {
    return !fieldType.isIsShortString() && (fieldType.getEnum_type() == FieldTypeEnum.STRING);
  }
}
