/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SRC_JITBUF_ID_MAPPING_H_
#define SRC_JITBUF_ID_MAPPING_H_

#include <google/protobuf/descriptor.h>
#include <platform/platform.h>

namespace jitbuf {

/**
 * Class representing a mapping of a single package
 */
class IdMapping {
public:
  /**
   * C'tor
   * @param file: the proto file that is being compiled.
   */
  IdMapping(const google::protobuf::FileDescriptor *file);

  /**
   * Maps the given package-internal RPC ID to a global RPC ID.
   */
  u32 map(u32 n);

private:
  /* interval starts */
  std::vector<u32> first;
  /* interval ends (inclusive) */
  std::vector<u32> last;
};

} // namespace jitbuf

#endif /* SRC_JITBUF_ID_MAPPING_H_ */
