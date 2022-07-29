// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.extensions

import net.flowmill.render.render.FieldType
import net.flowmill.render.render.FieldTypeEnum

class FieldTypeExtensions {

	static def cType(FieldType fieldType, boolean packedStrings)
	{
		switch(fieldType.enum_type) {
		case FieldTypeEnum.U8:	"uint8_t"
		case FieldTypeEnum.U16:	"uint16_t"
		case FieldTypeEnum.U32:	"uint32_t"
		case FieldTypeEnum.U64:	"uint64_t"
		case FieldTypeEnum.U128: "unsigned __int128"
		case FieldTypeEnum.S8:	"int8_t"
		case FieldTypeEnum.S16:	"int16_t"
		case FieldTypeEnum.S32:	"int32_t"
		case FieldTypeEnum.S64:	"int64_t"
		case FieldTypeEnum.S128:	"__int128"
		case FieldTypeEnum.STRING :
			if (packedStrings)
				"uint16_t"
			else
				"struct jb_blob"
		}
	}

	static def cType(FieldType type, int array_size /* -1 if not array */) {
		val non_array_type =
			if (type.isShortString) {
				'''short_string<«type.size»>'''
			} else {
				type.enum_type.literal
			}

		if (array_size >= 0) {
			'''std::array<«non_array_type»,«array_size»>'''
		} else {
			non_array_type
		}
	}

	static def wireCType(FieldType fieldType) {
		fieldType.cType(true)
	}

	static def parsedCType(FieldType fieldType) {
		fieldType.cType(false)
	}

	static def size(FieldType fieldType, boolean packedStrings) {
		if (fieldType.isIsShortString)
			fieldType.size
		else switch(fieldType.enum_type) {
			case FieldTypeEnum.U8:	1
			case FieldTypeEnum.U16:	2
			case FieldTypeEnum.U32:	4
			case FieldTypeEnum.U64:	8
			case FieldTypeEnum.U128:	16
			case FieldTypeEnum.S8:	1
			case FieldTypeEnum.S16:	2
			case FieldTypeEnum.S32:	4
			case FieldTypeEnum.S64:	8
			case FieldTypeEnum.S128:	16
			case FieldTypeEnum.STRING :
				if (packedStrings)
					2
				else
					16
		}
	}

	static def wireSize(FieldType fieldType) {
		fieldType.size(true)
	}

	static def parsedSize(FieldType fieldType) {
		fieldType.size(false)
	}

	/**
	 * Get field alignment
	 */
	static def alignment(FieldType fieldType, boolean packedStrings) {
		if (fieldType.isIsShortString)
			1
		else switch(fieldType.enum_type) {
			case FieldTypeEnum.U8:	1
			case FieldTypeEnum.U16:	2
			case FieldTypeEnum.U32:	4
			case FieldTypeEnum.U64:	8
			case FieldTypeEnum.U128:	16
			case FieldTypeEnum.S8:	1
			case FieldTypeEnum.S16:	2
			case FieldTypeEnum.S32:	4
			case FieldTypeEnum.S64:	8
			case FieldTypeEnum.S128:	16
			case FieldTypeEnum.STRING:
				if (packedStrings)
					2
				else
					8
		}
	}

	static def wireAlignment(FieldType fieldType) {
		fieldType.alignment(true)
	}

	static def parsedAlignment(FieldType fieldType) {
		fieldType.alignment(false)
	}

	static def isSigned(FieldType field_type) {
		switch (field_type.enum_type) {
		case FieldTypeEnum.S8,
		case FieldTypeEnum.S16,
		case FieldTypeEnum.S32,
		case FieldTypeEnum.S64,
		case FieldTypeEnum.S128:
			true
		default:
			false
		}
	}

	static def isInt(FieldType field_type) {
		return (!field_type.isShortString) &&
				(field_type.enum_type != FieldTypeEnum.STRING)
	}

}
