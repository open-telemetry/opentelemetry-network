/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_TRANSFORMER_H_
#define INCLUDE_JITBUF_TRANSFORMER_H_

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>
#include <stdint.h>

namespace jitbuf {

typedef uint32_t u32;
typedef uint64_t u64;
typedef uint16_t (*transform)(const char *src, char *dst);

class Transform {
public:
  Transform() : xform_(NULL) {}

  Transform(transform xform, std::shared_ptr<llvm::ExecutionEngine> engine) : xform_(xform), engine_(engine) {}

  inline bool empty() { return (xform_ == NULL); }

  inline uint16_t operator()(const char *src, char *dst) { return xform_(src, dst); }

  inline bool operator==(const Transform &other) { return xform_ == other.xform_; }

  /**
   * gets the function pointer. note its lifetime can only be trusted as a
   *   subset of this Transform's lifetime.
   */
  inline transform get() { return xform_; }

private:
  transform xform_;
  std::shared_ptr<llvm::ExecutionEngine> engine_;
};

/**
 * Initializes global LLVM structures.
 * @throws exception on error
 */
void initialize_llvm();

struct BlobDetails {
  /* offset to the length of the blob, in the source message */
  u32 src_pos;

  /* offset to the struct jb_blob in the destination message */
  u32 dst_pos;

  /* true if dst_len_pos is valid and should be populated, false otherwise */
  bool should_write;

  /* the length of this field is all the way from offset to the
   *   src_msg->_len */
  bool length_is_remainder;
};

class Transformer {
public:
  Transformer(llvm::LLVMContext &context);

  /**
   * Returns a function that transforms buffers
   * @param src_pos: the positions of variables in the source array
   * @param dst_pos: the positions of variables in the destination array
   * @param sizes: variable sizes, must be in {1,2,4,8}
   * @param n_elem: the number of elements to transform
   * @param len_pos: location of the u16 total message length in src
   * @param src_size: size of the constant part of the source message
   * @param blobs: collection of BlobDetails for recipt of decoding blobs
   *
   * The generated function returns the length of the message. If
   *   len_pos < src_size or there is a length_is_remainder==true blob, it
   *   returns *(u16*)&src_msg[len_pos]. Otherwise, it returns src_size.
   */
  Transform
  get_xform(u32 *src_pos, u32 *dst_pos, u32 *sizes, u32 n_elem, u32 len_pos, u32 src_size, std::vector<BlobDetails> &blobs);

private:
  /**
   * generates a unique name from the prefix
   */
  std::string generate_unique_name(const char *prefix);

  /**
   * Returns a new module for JIT
   */
  llvm::Module *get_new_module();

  /**
   * Returns a memcpy function
   */
  llvm::Function *get_memcpy(llvm::Module *module);

  /**
   * JIT compiles the function in the given module, returns a pointer to it
   */
  Transform jit_function(llvm::Module *module, llvm::Function *function);

  u64 unique_counter_; /* counter for generating names */
  llvm::LLVMContext &context_;
  llvm::IRBuilder<> builder_;
};

} /* namespace jitbuf */

#endif /* INCLUDE_JITBUF_TRANSFORMER_H_ */
