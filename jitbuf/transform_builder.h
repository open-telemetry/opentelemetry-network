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

#ifndef INCLUDE_JITBUF_TRANSFORM_BUILDER_H_
#define INCLUDE_JITBUF_TRANSFORM_BUILDER_H_

#include <jitbuf/descriptor.h>
#include <jitbuf/jb.h>
#include <jitbuf/transformer.h>
#include <memory>
#include <stdint.h>
#include <unordered_map>

namespace llvm {
class LLVMContext;
}

namespace jitbuf {

/**
 * Handling incoming buffers including their mapping into known structs
 * @param min_buffer_size: the minimum required buffer size to be able to decode
 */
struct TransformRecord {
  uint32_t msg_rpc_id;
  Transform xform;
  uint32_t size;
  uint32_t min_buffer_size;
};

/**
 * The TransformBuilder creates functions that translate from the remote's
 *   message format to the local message format.
 *
 * The TransformBuilder is:
 *   1. Initialized with the local known message formats in the form of
 *      - descriptors -- via add_descriptor()
 *      - packages -- via add_package()
 *   2. Then queried for the functions via get_xform(), given the serialized
 *     representation of the remote's message format.
 *
 *  The included Transformer (xformer_) handles the LLVM-related parts of
 *    creating the functions.
 */
class TransformBuilder {
public:
  TransformBuilder(llvm::LLVMContext &context);

  /**
   * Adds a message descriptor.
   */
  void add_descriptor(std::shared_ptr<Descriptor> descriptor);

  /**
   * Adds a message descriptor.
   */
  void add_descriptor(const std::string &descriptor);

  /**
   * Gets a buffer handler, given the serialized JitbufDescriptor.
   */
  std::shared_ptr<TransformRecord> get_xform(const std::string &from);

  /**
   * Returns a function that transforms between two jitbuf descriptors, and
   *   the minimum incoming message for safe operation
   * @param from: the descriptor of the source message
   * @param to: the descriptor of the destination message
   */
  std::shared_ptr<TransformRecord> get_xform_to(const Descriptor &from, const Descriptor &to);

private:
  Transformer xformer_;

  std::unordered_map<uint32_t, std::shared_ptr<Descriptor>> descriptors_;
  std::unordered_map<std::string, std::weak_ptr<TransformRecord>> cache_;
};

} /* namespace jitbuf */

#endif /* INCLUDE_JITBUF_TRANSFORM_BUILDER_H_ */
