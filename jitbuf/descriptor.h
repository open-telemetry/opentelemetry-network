/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>
#include <stdexcept>
#include <vector>

namespace jitbuf {

struct Field {
  /* field's id */
  u16 field_id;

  /* ftype */
  enum class ftype_t { INT8, INT16, INT32, INT64, VAR, INT128 } ftype;

  /* array size. 1 can mean array of size 1 or non-array. */
  u16 n_elems;

  /* position, valid only after compute_positions() */
  u16 pos;

  /**
   * @return the size of the field
   * @param packed_strings: whether strings are encoded at the end of the
   *   fixed message
   */
  inline u16 size(bool packed_strings) const;
};

struct Descriptor {
  /* the rpc id of the message */
  u16 rpc_id;

  /* the fields */
  std::vector<Field> fields;

  /* is this message dynamically sized */
  bool dynamic_size;

  /* total size of the message's fixed part (without packed strings) */
  u16 size;

  /* number of VAR fields */
  u16 n_var_fields;
};

} /* namespace jitbuf */

/*************
 * impl
 */
u16 jitbuf::Field::size(bool packed_strings) const
{
  switch (ftype) {
  case Field::ftype_t::INT8:
    return 1;
  case Field::ftype_t::INT16:
    return 2;
  case Field::ftype_t::INT32:
    return 4;
  case Field::ftype_t::INT64:
    return 8;
  case Field::ftype_t::VAR:
    return (packed_strings ? 2 : 4);
  case Field::ftype_t::INT128:
    return 16;
  }

  throw std::runtime_error("unknown ftype");
}
