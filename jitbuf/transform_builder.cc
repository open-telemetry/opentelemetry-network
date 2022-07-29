// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <jitbuf/transform_builder.h>

#include <jitbuf/descriptor_reader.h>
#include <sstream>
#include <vector>

namespace jitbuf {

TransformBuilder::TransformBuilder(llvm::LLVMContext &context) : xformer_(context) {}

void TransformBuilder::add_descriptor(std::shared_ptr<Descriptor> descriptor)
{
  u32 struct_rpc_id = descriptor->rpc_id;

  auto res = descriptors_.insert({struct_rpc_id, descriptor});
  if (res.second != true) {
    std::stringstream msg;
    msg << "rpc_id " << struct_rpc_id << " already exists";
    throw std::runtime_error(msg.str());
  }

  return;
}

void jitbuf::TransformBuilder::add_descriptor(const std::string &descriptor)
{
  std::shared_ptr<Descriptor> desc(new Descriptor(DescriptorReader::read((u8 *)descriptor.data(), descriptor.size())));

  DescriptorReader::compute_positions(*desc, false);

  add_descriptor(desc);
}

std::shared_ptr<TransformRecord> TransformBuilder::get_xform(const std::string &from)
{
  /* first try the cache */
  auto iter = cache_.find(from);
  if (iter != cache_.end()) {
    /* found! try to get a strong pointer from the weak pointer */
    auto sptr = iter->second.lock();
    if (sptr)
      return sptr;

    /* buffer has been freed, will fall through and make a new one */
    cache_.erase(iter);
  }

  /* De-serialize the descriptor */
  Descriptor jb_desc(DescriptorReader::read((u8 *)from.data(), from.size()));

  DescriptorReader::compute_positions(jb_desc, true);

  u32 struct_rpc_id = jb_desc.rpc_id;

  /* find the struct handler for the rpc_id. might throw std::out_of_bounds */
  auto to_ptr = descriptors_.at(struct_rpc_id);

  /* make a MessageHandler */
  std::shared_ptr<TransformRecord> buf(get_xform_to(jb_desc, *to_ptr));

  /* put in the cache */
  cache_.insert({from, buf});

  return buf;
}

std::shared_ptr<TransformRecord> TransformBuilder::get_xform_to(const Descriptor &from, const Descriptor &to)
{
  std::vector<u32> src_pos;
  std::vector<u32> dst_pos;
  std::vector<u32> sizes;
  uint32_t min_size = 0;
  u32 len_pos = 0xffffffff;
  const u32 src_size = from.size;
  std::vector<BlobDetails> blobs;

  /** regular fields */
  /* how many fields does each message type contain? */
  int from_fields = from.fields.size();
  int to_fields = to.fields.size();
  int max_fields = std::min(from_fields, to_fields);

  /* populate a map [id] -> [pos] of the non-string @from fields */
  std::map<u32, u32> from_map;
  for (auto &field : from.fields)
    if (field.ftype != Field::ftype_t::VAR)
      from_map[field.field_id] = field.pos;

  /* reserve to avoid unnecessary re-allocations */
  src_pos.reserve(max_fields);
  dst_pos.reserve(max_fields);
  sizes.reserve(max_fields);

  /* for each @to field, try to match with a @from field */
  for (auto &to_field : to.fields) {
    /* this pass: only non-VAR fields */
    if (to_field.ftype == Field::ftype_t::VAR)
      continue;

    u32 field_id = to_field.field_id;
    auto from_field_it = from_map.find(field_id);
    if (from_field_it == from_map.end())
      continue; /* no field in @from */

    src_pos.push_back(from_field_it->second);
    dst_pos.push_back(to_field.pos);
    u32 field_size = to_field.size(false) * to_field.n_elems;
    sizes.push_back(field_size);
    min_size = std::max(min_size, from_field_it->second + field_size);
  }

  /* sanity checks */
  if (min_size > src_size)
    throw std::runtime_error("descriptor fields must be smaller than incoming total size");

  /** VAR fields */
  int from_blobs = from.n_var_fields;
  int to_blobs = to.n_var_fields;

  if (from_blobs > 0)
    len_pos = 2;

  if ((from_blobs > 0) && (to_blobs > 0)) {

    /* populate a map [id] -> [pos] of the @to blobs */
    std::map<u32, u32> to_map;
    for (auto &to_field : to.fields)
      if (to_field.ftype == Field::ftype_t::VAR)
        to_map[to_field.field_id] = to_field.pos;

    /* reserve space to avoid re-allocation */
    blobs.reserve(from_blobs);

    /* we'll remember the last should_write = true */
    int last_should_write = -1;

    /* for each @from blob, add a BlobDetails */
    int i = 0;
    for (auto &from_field : from.fields) {
      if (from_field.ftype != Field::ftype_t::VAR)
        continue;

      BlobDetails blob;

      /* fill in the from pos */
      if (i == from.n_var_fields - 1) {
        /* last blob does not have src; its length is the remainder */
        blob.length_is_remainder = true;
      } else {
        blob.src_pos = from_field.pos;
        blob.length_is_remainder = false;
      }

      /* try to find a matching to blob */
      u32 field_id = from_field.field_id;
      auto to_field_it = to_map.find(field_id);
      if (to_field_it == to_map.end()) {
        /* no field in @to */
        blob.should_write = false;
      } else {
        blob.dst_pos = to_field_it->second;
        blob.should_write = true;
        last_should_write = i;

        /* leverage the field copy code to copy the len if we're
         * not the from field */
        if (!blob.length_is_remainder) {
          src_pos.push_back(blob.src_pos);
          dst_pos.push_back(blob.dst_pos + sizeof(char *));
          sizes.push_back(sizeof(uint16_t));
        }
      }

      blobs.push_back(blob);
      i++;
    }

    /* we've pushed back a blob for each from blob, but if at some point
     * there are no more @to blocks we need to write to, why bother going
     * through all those from blobs? In this case we can throw away the last
     * few blobs.
     */
    blobs.resize(last_should_write + 1); /* 0 if no to blobs matched */
  }

  /* jit-compile the transformation */
  std::shared_ptr<TransformRecord> buf(new TransformRecord);
  buf->msg_rpc_id = to.rpc_id;
  buf->xform = xformer_.get_xform(src_pos.data(), dst_pos.data(), sizes.data(), sizes.size(), len_pos, src_size, blobs);
  buf->size = src_size;
  buf->min_buffer_size = min_size;

  return buf;
}

} /* namespace jitbuf */
