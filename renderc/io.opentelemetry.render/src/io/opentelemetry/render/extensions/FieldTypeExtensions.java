// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import com.google.common.base.Objects;
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
          String expression = null;
          if (packedStrings) {
            expression = "uint16_t";
          } else {
            expression = "struct jb_blob";
          }
          switchResult = expression;
          break;
        default:
          break;
      }
    }
    return switchResult;
  }

  public static CharSequence cType(final FieldType type, final int arraySize) {
    CharSequence blockExpression = null;
    {
      String expression = null;
      boolean shortString = type.isIsShortString();
      if (shortString) {
        StringBuilder builder = new StringBuilder();
        builder.append("short_string<");
        int size = type.getSize();
        builder.append(size);
        builder.append(">");
        expression = builder.toString();
      } else {
        expression = type.getEnum_type().getLiteral();
      }
      final String nonArrayType = expression;
      CharSequence expression1 = null;
      if (arraySize >= 0) {
        StringBuilder builder1 = new StringBuilder();
        builder1.append("std::array<");
        builder1.append(nonArrayType);
        builder1.append(",");
        builder1.append(arraySize);
        builder1.append(">");
        expression1 = builder1;
      } else {
        expression1 = nonArrayType;
      }
      blockExpression = expression1;
    }
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
    boolean shortString = fieldType.isIsShortString();
    if (shortString) {
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
            int expression1 = (int) 0;
            if (packedStrings) {
              expression1 = 2;
            } else {
              expression1 = 16;
            }
            switchResult = expression1;
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
    boolean shortString = fieldType.isIsShortString();
    if (shortString) {
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
            int expression1 = (int) 0;
            if (packedStrings) {
              expression1 = 2;
            } else {
              expression1 = 8;
            }
            switchResult = expression1;
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
    return ((!fieldType.isIsShortString()) && (fieldType.getEnum_type() == FieldTypeEnum.STRING));
  }
}