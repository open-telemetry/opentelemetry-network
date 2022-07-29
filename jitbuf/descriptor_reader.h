/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jitbuf/descriptor.h>
#include <platform/types.h>

namespace jitbuf {

/*
 * Jitbuf message descriptor:
 *
 * All fields are 2-bytes aligned to 2 bytes.
 *
 * HEADER:
 * +-----------+------------+------------+------------+
 * |   FLAGS   |   RPC_ID   |  N_FIELDS  |  N_ARRAYS  |
 * +-----------+------------+------------+------------+
 *
 * BODY:
 * +------------+---------+---------------------+
 * |  FIELD[0]  |   ...   | FIELD[N_FIELDS - 1] |
 * +------------+---------+---------------------+
 * +------------+---------+---------------------+
 * |   ARR[0]   |   ...   |   ARR[N_ARRAYS-1]   |
 * +------------+---------+---------------------+
 *
 * Where:
 * FLAGS is 0
 * RPC_ID is the described message's rpc id
 * N_FIELDS counts the number of fields encoded in the message
 * N_ARRAYS is the number of array fields specified
 *
 * FIELD:
 * 16       15             12                                  0
 *  +--------+----+----+----+----------------------------------+
 *  | IS_ARR |    FTYPE     |             FIELD ID             |
 *  +--------+----+----+----+----------------------------------+
 *
 * IS_ARR: 1 if the field is an array, so is specified in the ARR part
 * FTYPE: specifies the size type of field of the following values:
 *   0: 1 bytes (u8 or s8)
 *   1: 2 bytes (u16 or s16)
 *   2: 4 bytes (u32 or s32)
 *   3: 8 bytes (u64 or s64)
 *   4: variable length string.
 *        When strings are packed, length is specified with a u16, except the
 *          last string, whose length is deduced from the message's total length
 *        When strings are unpacked, this is represented by a struct jb_blob
 *          (see jb.h)
 * FIELD_ID: the field ID of the field in the message specification
 *
 * ARR: number of elements in the array. Currently required 4096 > ARR > 0.
 */

class DescriptorReader {
public:
  /**
   * Read descriptor buffer, return Descriptor.
   *
   * @assumes buffer is 16-bit aligned in memory
   * @throws on error.
   */
  static Descriptor read(u8 *buffer, u16 len);

  /**
   * Computes the positions of message fields
   *
   * @param packed_strings: true if only string lengths are
   */
  static void compute_positions(Descriptor &descriptor, bool packed_strings);
};

} /* namespace jitbuf */
