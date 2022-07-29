// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <jitbuf/descriptor_reader.h>

#include <assert.h>
#include <stdexcept>

namespace jitbuf {

Descriptor DescriptorReader::read(u8 *buffer, u16 len)
{
  if (len < 8)
    throw std::runtime_error("descriptor should be >= 8 bytes");

  u16 *buf16 = (u16 *)buffer;

  /* unpack header */
  u16 flags = buf16[0];
  u16 rpc_id = buf16[1];
  u16 n_fields = buf16[2];
  u16 n_arrays = buf16[3];

  /* sanity check header */
  assert(flags == 0);
  (void)flags; /* for unused warning */

  if (len < 8 + 2 * n_fields + 2 * n_arrays)
    throw std::runtime_error("descriptor too small for n_fields+n_arrays");

  if (n_arrays > n_fields)
    throw std::runtime_error("n_arrays must not be larger than n_fields");

  /* make return Descriptor */
  Descriptor res;
  res.rpc_id = rpc_id;
  res.fields.resize(n_fields);

  /* convenience pointers to start of fields and arrays */
  u16 *fields = &buf16[4];
  u16 *arrays = &buf16[4 + n_fields];

  /* array_index is the index of the next array field */
  u16 array_index = 0;

  /* read fields */
  for (int i = 0; i < n_fields; i++) {
    u16 field = fields[i];

    /* unpack the field */
    bool is_arr = field >> 15;
    u16 ftype16 = (field >> 12) & 0x7;
    u16 field_id = field & 0x0FFF;

    /* sanity checks */
    if (is_arr) {
      if (array_index >= n_arrays)
        throw std::runtime_error("descriptor has more than n_arrays fields marked as arrays");
      if ((arrays[array_index] == 0) || (arrays[array_index] > 4096))
        throw std::runtime_error("array size out of bounds");
    }

    /* read the field */
    res.fields[i].field_id = field_id;
    switch (ftype16) {
    case 0:
      res.fields[i].ftype = Field::ftype_t::INT8;
      break;
    case 1:
      res.fields[i].ftype = Field::ftype_t::INT16;
      break;
    case 2:
      res.fields[i].ftype = Field::ftype_t::INT32;
      break;
    case 3:
      res.fields[i].ftype = Field::ftype_t::INT64;
      break;
    case 4:
      res.fields[i].ftype = Field::ftype_t::VAR;
      break;
    case 5:
      res.fields[i].ftype = Field::ftype_t::INT128;
      break;
    default:
      throw std::runtime_error("unknown ftype");
    }
    res.fields[i].n_elems = (is_arr ? arrays[array_index++] : 1);
  }

  return res;
}

void DescriptorReader::compute_positions(Descriptor &descriptor, bool packed_strings)
{
  /* is this message dynamically sized? */
  u16 n_var_fields = 0;
  for (auto &field : descriptor.fields)
    if (field.ftype == Field::ftype_t::VAR)
      n_var_fields++;

  /* note: if packed_strings == false, then dynamic_size is false. */
  descriptor.n_var_fields = n_var_fields;
  descriptor.dynamic_size = packed_strings && (n_var_fields > 0);
  u16 pos = (descriptor.dynamic_size) ? 4 : 2;

  /* n_var_seen counts the number of VAR fields iterated through, so
   * we can ignore the last VAR field (in favor of the total length field)
   */
  int n_var_seen = 0;

  for (auto &field : descriptor.fields) {
    u16 size = field.size(packed_strings);
    u16 align = ((field.ftype == Field::ftype_t::VAR) ? 2 : size);

    if (packed_strings && (field.ftype == Field::ftype_t::VAR)) {
      n_var_seen++;
      if (n_var_seen == n_var_fields) {
        // don't position the last dynamic field
        field.pos = 0;
        continue;
      }
    }

    /* align for the value according to alignement */
    pos = (pos + align - 1) & (~(align - 1));

    /* set the position in the Field struct */
    field.pos = pos;

    /* update the next position after this field. */
    pos += size * field.n_elems;
  }

  descriptor.size = pos;
}

} /* namespace jitbuf */
