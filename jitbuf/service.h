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
