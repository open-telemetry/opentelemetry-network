/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_SERVICE_H_
#define INCLUDE_JITBUF_SERVICE_H_

#include <jitbuf/jb.h>
#include <jitbuf/transformer.h>

namespace jitbuf {

typedef uint16_t (*handler_func_t)(const char *msg, Transform &xform, void *priv);

/**
 * Information required to construct a single message handler
 */
struct handler_descriptor {
  const struct jb_descriptor *descriptor;
  handler_func_t handler_func;
};

/**
 * Information required to construct all of a package's handlers
 */
struct handler_package {
  const struct handler_descriptor *descriptors;
  int size;
};

class Service {
public:
  virtual ~Service() {}
  virtual const handler_package get_package() { throw std::runtime_error("unimplemented"); }
};

} // namespace jitbuf

#endif /* INCLUDE_JITBUF_SERVICE_H_ */
