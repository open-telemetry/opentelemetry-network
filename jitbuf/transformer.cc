//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <jitbuf/transformer.h>

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define SHOULD_DUMP_MODULES_AFTER_JIT 0

namespace jitbuf {

void initialize_llvm()
{
  bool res;
  res = llvm::InitializeNativeTarget();
  if (res)
    throw std::runtime_error("llvm::InitializeNativeTarget() failed");

  res = llvm::InitializeNativeTargetAsmPrinter();
  if (res)
    throw std::runtime_error("llvm::InitializeNativeTargetAsmPrinter() failed");

  res = llvm::InitializeNativeTargetAsmParser();
  if (res)
    throw std::runtime_error("llvm::InitializeNativeTargetAsmParser() failed");
}

Transformer::Transformer(llvm::LLVMContext &context) : unique_counter_(0), context_(context), builder_(context) {}

Transform Transformer::get_xform(
    u32 *src_pos, u32 *dst_pos, u32 *sizes, u32 n_elem, u32 len_pos, u32 src_size, std::vector<BlobDetails> &blobs)
{
  llvm::IntegerType *U16T = llvm::Type::getInt16Ty(context_);
  llvm::IntegerType *U32T = llvm::Type::getInt32Ty(context_);
  llvm::PointerType *S8PT = llvm::Type::getInt1PtrTy(context_);

  llvm::Function *memcpy_func = NULL;

  /* arguments are two char[] */
  std::vector<llvm::Type *> args(2, llvm::Type::getInt8PtrTy(context_, 0));

  /* returns u16 */
  llvm::FunctionType *func_type = llvm::FunctionType::get(U16T, args, false);
  llvm::Value *ret_value = NULL;

  /* get a module as a container */
  llvm::Module *module = get_new_module();

  /* create prototype */
  std::string func_name = generate_unique_name("xform_");
  llvm::Function *F = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, module);

  /* set names and get Value* for function parameters */
  llvm::Function::arg_iterator AI = F->arg_begin();
  AI->setName("src");
  llvm::Value *src_buf_ptr = &(*(AI++));
  AI->setName("dst");
  llvm::Value *dst_buf_ptr = &(*(AI++));

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(context_, "entry", F);
  builder_.SetInsertPoint(BB);

  /* Add IR to copy fields from src to dst */
  for (u32 i = 0; i < n_elem; i++) {
    /* get constant for source position */
    llvm::Value *src_pos_val = llvm::ConstantInt::get(U32T, src_pos[i], false);
    /* get constant for destination position */
    llvm::Value *dst_pos_val = llvm::ConstantInt::get(U32T, dst_pos[i], false);

    /* Pointer to source variable */
    llvm::Value *src_char_ptr = builder_.CreateGEP(src_buf_ptr, src_pos_val, "src_charp");
    /* Pointer to dst variable */
    llvm::Value *dst_char_ptr = builder_.CreateGEP(dst_buf_ptr, dst_pos_val, "dst_charp");

    /* decide which type the value is */
    llvm::Type *copied_type;
    switch (sizes[i]) {
    case 1:
      copied_type = llvm::Type::getInt8Ty(context_);
      break;
    case 2:
      copied_type = llvm::Type::getInt16Ty(context_);
      break;
    case 4:
      copied_type = llvm::Type::getInt32Ty(context_);
      break;
    case 8:
      copied_type = llvm::Type::getInt64Ty(context_);
      break;
    case 16:
      copied_type = llvm::Type::getInt128Ty(context_);
      break;
    default:
      copied_type = NULL;
      break;
    }

    u32 is_unaligned = (src_pos[i] | dst_pos[i]) & (sizes[i] - 1);

    if ((copied_type != NULL) && (is_unaligned == 0)) {
      /* cast pointers to correct type */
      llvm::Value *src_ptr = builder_.CreatePointerCast(src_char_ptr, copied_type->getPointerTo(), "src_ptr");
      llvm::Value *dst_ptr = builder_.CreatePointerCast(dst_char_ptr, copied_type->getPointerTo(), "dst_ptr");

      /* Load the source */
      llvm::LoadInst *loaded = builder_.CreateLoad(copied_type, src_ptr, "ld");
      /* Save to destination */
      llvm::StoreInst *stored = builder_.CreateStore(loaded, dst_ptr);
      (void)stored; /* unused, only need the side effect */
    } else {
      /* pointers are unaligned or don't have a suitable register size */
      /* get memcpy */

      if (memcpy_func == NULL)
        memcpy_func = get_memcpy(module);

      /* get constant for size */
      llvm::Value *size_val = llvm::ConstantInt::get(U32T, sizes[i], true);

      std::vector<llvm::Value *> memcpy_args;
      memcpy_args.push_back(dst_char_ptr);
      memcpy_args.push_back(src_char_ptr);
      memcpy_args.push_back(size_val);

      builder_.CreateCall(memcpy_func, memcpy_args, "memcpy_ret");
    }
  }

  /* Translate blobs */
  if (blobs.size() > 0) {
    /* our initial offset is the message size */
    llvm::Value *offset = llvm::ConstantInt::get(U16T, src_size, false);

    for (auto &blob : blobs) {
      if (blob.should_write) {
        /* get constant for destination position */
        llvm::Value *dst_pos_val = llvm::ConstantInt::get(U32T, blob.dst_pos, false);

        /* Char pointer to dst variable */
        llvm::Value *dst_char_ptr = builder_.CreateGEP(dst_buf_ptr, dst_pos_val, "offset_charp");

        /* cast pointer to correct type */
        llvm::Value *dst_ptr = builder_.CreatePointerCast(dst_char_ptr, S8PT->getPointerTo(), "buf_ptr");

        /* compute &src[offset] */
        llvm::Value *src_blob_ptr = builder_.CreateGEP(src_buf_ptr, offset, "src_blob_ptr");

        /* Save to destination */
        llvm::StoreInst *stored = builder_.CreateStore(src_blob_ptr, dst_ptr);
        (void)stored; /* unused, only need the side effect */
      }

      /* are we writing the last @from field? */
      if (blob.should_write && blob.length_is_remainder) {
        /* get constant for source position */
        llvm::Value *src_pos_val = llvm::ConstantInt::get(U32T, len_pos, false);

        /* Pointer to source variable */
        llvm::Value *src_char_ptr = builder_.CreateGEP(src_buf_ptr, src_pos_val, "total_len_charp");

        /* cast pointers to correct type */
        llvm::Value *src_ptr = builder_.CreatePointerCast(src_char_ptr, U16T->getPointerTo(), "total_len_ptr");

        /* load */
        llvm::LoadInst *loaded = builder_.CreateLoad(U16T, src_ptr, "loaded_total");

        /* save this for return from the generated function */
        ret_value = loaded;

        /* get constant for destination position */
        llvm::Value *dst_pos_val = llvm::ConstantInt::get(U32T, blob.dst_pos + sizeof(uint16_t), false);

        /* Char pointer to dst variable */
        llvm::Value *dst_char_ptr = builder_.CreateGEP(dst_buf_ptr, dst_pos_val, "last_len_charp");

        /* cast pointer to correct type */
        llvm::Value *dst_ptr = builder_.CreatePointerCast(dst_char_ptr, U16T->getPointerTo(), "last_len_ptr");

        /* compute total - offset */
        llvm::Value *remaining_val = builder_.CreateSub(loaded, offset, "remaining_len");

        /* Save to destination */
        llvm::StoreInst *stored = builder_.CreateStore(remaining_val, dst_ptr);
        (void)stored; /* unused, only need the side effect */
      }

      /* update the offset */
      if (!blob.length_is_remainder) {
        /* get constant for source position */
        llvm::Value *src_pos_val = llvm::ConstantInt::get(U32T, blob.src_pos, false);

        /* Pointer to source variable */
        llvm::Value *src_char_ptr = builder_.CreateGEP(src_buf_ptr, src_pos_val, "field_len_charp");

        /* cast pointers to correct type */
        llvm::Value *src_ptr = builder_.CreatePointerCast(src_char_ptr, U16T->getPointerTo(), "field_len_ptr");

        /* load */
        llvm::LoadInst *loaded = builder_.CreateLoad(U16T, src_ptr, "field_len");

        /* update offset */
        offset = builder_.CreateAdd(offset, loaded, "cur_offset");
      }
    }
  }

  // Decide on return value if we haven't already
  if (ret_value == NULL) {
    if (len_pos <= src_size - sizeof(uint16_t)) {
      /* dynamic length -- duplicated code from blob handling :/ */
      /* get constant for source position */
      llvm::Value *src_pos_val = llvm::ConstantInt::get(U32T, len_pos, false);

      /* Pointer to source variable */
      llvm::Value *src_char_ptr = builder_.CreateGEP(src_buf_ptr, src_pos_val, "total_len_charp");

      /* cast pointers to correct type */
      llvm::Value *src_ptr = builder_.CreatePointerCast(src_char_ptr, U16T->getPointerTo(), "total_len_ptr");

      /* load */
      llvm::LoadInst *loaded = builder_.CreateLoad(U16T, src_ptr, "loaded_total");

      /* we'll return that loaded variable */
      ret_value = loaded;
    } else {
      /* our initial offset is the message size */
      ret_value = llvm::ConstantInt::get(U16T, src_size, false);
    }
  }

  // Finish off the function.
  builder_.CreateRet(ret_value);

  // Validate the generated code, checking for consistency.
  verifyFunction(*F);

  /* For debugging */
  if (SHOULD_DUMP_MODULES_AFTER_JIT)
    module->dump();

  return jit_function(module, F);
}

std::string Transformer::generate_unique_name(const char *prefix)
{
  std::stringstream st;
  st << prefix << unique_counter_++;
  return st.str();
}

llvm::Module *Transformer::get_new_module()
{
  // Otherwise create a new Module.
  std::string module_name = generate_unique_name("mcjit_module_");
  llvm::Module *m = new llvm::Module(module_name, context_);
  return m;
}

llvm::Function *Transformer::get_memcpy(llvm::Module *module)
{
  /* create arguments */
  std::vector<llvm::Type *> args(2, llvm::Type::getInt8PtrTy(context_, 0));
  args.push_back(llvm::Type::getInt32Ty(context_));

  /* return type */
  llvm::Type *return_type = llvm::Type::getInt8PtrTy(context_, 0);

  /* function type */
  llvm::FunctionType *func_type = llvm::FunctionType::get(return_type, args, false);

  /* function */
  llvm::Function *memcpy_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "memcpy", module);

  if (memcpy_func == NULL)
    throw std::runtime_error("could not create memcpy func");

  return memcpy_func;
}

Transform Transformer::jit_function(llvm::Module *module, llvm::Function *function)
{
  /* Make a new ExecutionEngine */
  std::string error_str;
  std::shared_ptr<llvm::ExecutionEngine> engine(llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module))
                                                    .setErrorStr(&error_str)
                                                    .setEngineKind(llvm::EngineKind::JIT)
                                                    .create());
  if (!engine) {
    std::stringstream st;
    st << "Could not create ExecutionEngine: " << error_str;
    throw std::runtime_error(st.str());
  }

  // Create a function pass manager for this engine
  auto *pass_manager = new llvm::legacy::FunctionPassManager(module);

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  module->setDataLayout(engine->getDataLayout());
  // Provide basic AliasAnalysis support for GVN.
  pass_manager->add(llvm::createBasicAAWrapperPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  pass_manager->add(llvm::createInstructionCombiningPass());
  // Reassociate expressions.
  pass_manager->add(llvm::createReassociatePass());
  // Eliminate Common SubExpressions.
  pass_manager->add(llvm::createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  pass_manager->add(llvm::createCFGSimplificationPass());
  pass_manager->doInitialization();

  // Run the pass manager on this function
  pass_manager->run(*function);

  // We don't need this anymore
  delete pass_manager;

  module = NULL;
  engine->finalizeObject();
  return Transform((transform)engine->getPointerToFunction(function), engine);
}

} /* namespace jitbuf */
