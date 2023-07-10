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
          String xIfExpression = null;
          if (packedStrings) {
            xIfExpression = "uint16_t";
          } else {
            xIfExpression = "struct jb_blob";
          }
          switchResult = xIfExpression;
          break;
        default:
          break;
      }
    }
    return switchResult;
  }


 public static CharSequence cType(final FieldType type, final int arraySize) {
    CharSequence xBlockExpression = null;
    {
      String xIfExpression = null;
      boolean isItShortString = type.isIsShortString();
      if (isItShortString) {
        StringBuilder builder = new StringBuilder();
        builder.append("short_string<");
        int size = type.getSize();
        builder.append(size);
        builder.append(">");
        xIfExpression = builder.toString();
      } else {
        xIfExpression = type.getEnum_type().getLiteral();
      }
      final String nonArrayType = xIfExpression;
      CharSequence xIfExpression1 = null;
      if ((arraySize >= 0)) {
        StringBuilder builder1 = new StringBuilder();
        builder1.append("std::array<");
        builder1.append(nonArrayType);
        builder1.append(",");
        builder1.append(arraySize);
        builder1.append(">");
        xIfExpression1 = builder1;
      } else {
        xIfExpression1 = nonArrayType;
      }
      xBlockExpression = xIfExpression1;
    }
    return xBlockExpression;
  }
 public static String wireCType(final FieldType fieldType) {
    return FieldTypeExtensions.cType(fieldType, true);
  }

  public static String parsedCType(final FieldType fieldType) {
    return FieldTypeExtensions.cType(fieldType, false);
  }

  public static int size(final FieldType fieldType, final boolean packedStrings) {
    int xIfExpression;
    boolean isItShortString = fieldType.isIsShortString();
    if (isItShortString) {
      xIfExpression = fieldType.getSize();
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
            int xIfExpression1 = (int) 0;
            if (packedStrings) {
              xIfExpression1 = 2;
            } else {
              xIfExpression1 = 16;
            }
            switchResult = xIfExpression1;
            break;
          default:
            break;
        }
      }
xIfExpression = switchResult;
    }
    return xIfExpression;
  }

  public static int wireSize(final FieldType fieldType) {
    return FieldTypeExtensions.size(fieldType, true);
  }

    public static int parsedSize(final FieldType fieldType) {
    return FieldTypeExtensions.size(fieldType, false);
  }

  public static int alignment(final FieldType fieldType, final boolean packedStrings) {
    int xIfExpression;
    boolean isItShortString = fieldType.isIsShortString();
    if (isItShortString) {
      xIfExpression = 1;
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
            int xIfExpression1 = (int) 0;
            if (packedStrings) {
              xIfExpression1 = 2;
            } else {
              xIfExpression1 = 8;
            }
            switchResult = xIfExpression1;
            break;
 }
      }
      xIfExpression = switchResult;
    }
    return xIfExpression;
  }

  public static int wireAlignment(final FieldType fieldType) {
    return FieldTypeExtensions.alignment(fieldType, true);
  }

  public static int parsedAlignment(final FieldType fieldType) {
    return FieldTypeExtensions.alignment(fieldType, false);
  }
 public static boolean isInt(final FieldType fieldType) {
    return ((!fieldType.isIsShortString()) && (!Objects.equal(fieldType.getEnum_type(), FieldTypeEnum.STRING)));
  }
}





