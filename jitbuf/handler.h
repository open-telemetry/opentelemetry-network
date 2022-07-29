/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_HANDLER_H_
#define INCLUDE_JITBUF_HANDLER_H_

#include <jitbuf/service.h>
#include <jitbuf/transform_builder.h>
#include <unordered_map>

namespace jitbuf {

/**
 * A record for handling of messages
 * @transform: details of transforming the source RPC format to our format
 * @handler_func: a handler for out format messages
 * @priv: parameter to the handler func
 * @transform_ptr: keep the shared_ptr for accurate reference counting
 * @service_ptr: if applicable, keep the shared_ptr for the given service
 */
struct HandlerRecord {
  struct TransformRecord transform;
  handler_func_t handler_func;
  void *priv;
  std::shared_ptr<jitbuf::TransformRecord> transform_ptr;
  std::shared_ptr<jitbuf::Service> service_ptr;
};

/* Flags for message handling: */
/*   There is a timestamp before the message */
#define HFLAG_TIMESTAMPED 0x1

/* message handler: */
class Handler {
public:
  /**
   * c'tor
   * @param fail_unadded: should handle() fail on rpc_ids that were not added
   * @param fail_unknown: should handle() fail on rpc_ids that are not in the
   * .proto
   */
  Handler(bool fail_unadded, bool fail_unknown);

  /**
   * Add handling of new message
   * @param transform: the details of JIT transform from source RPC format
   * @param handler_func: the function to handle messages
   * @param service: a Service class to implement the service
   */
  void add(std::shared_ptr<jitbuf::TransformRecord> &transform, handler_func_t handler_func, std::shared_ptr<Service> service);

  /**
   * Add handling of new message
   * @param transform: the details of JIT transform from source RPC format
   * @param handler_func: the function to handle messages
   * @param priv: parameter to the function
   */
  void add(std::shared_ptr<jitbuf::TransformRecord> &transform, handler_func_t handler_func, void *priv);

  /**
   * Add identity handling of all messages in package
   * @param builder: a TransformBuilder to generate transforms
   * @param service: the service to add identity handlers for
   */
  void add_identity(TransformBuilder &builder, std::shared_ptr<Service> service);

  /**
   * Add identity handling of all messages in package. No smart-pointer variant
   *
   * @param builder: a TransformBuilder to generate transforms
   * @param service: the service to add identity handlers for
   */
  void add_identity(TransformBuilder &builder, Service *service);

  /**
   * handle a message
   * @returns: the message length on success, -EINVAL on error (or throws)
   */
  int handle(const char *msg, uint32_t len, u64 flags = 0);
  int handle(const std::string &msg, u64 flags = 0);

  /**
   * Handles multiple consecutive messages
   * @returns: the length of successfully consumed messages if at least one
   *           message was processed, otherwise returns -EINVAL (or throws)
   */
  int handle_multiple(const char *msg, u64 len, u64 flags = 0);
  int handle_multiple(const std::string &msg, u64 flags = 0);

  /* get the most recently known remote timestamp */
  inline u64 remote_timestamp() { return remote_timestamp_; }

  /* accessors to individual transforms */
  jitbuf::Transform get_transform(int rpc_id);

private:
  bool fail_unadded_;
  bool fail_unknown_;

  u64 remote_timestamp_;

  std::unordered_map<uint32_t, HandlerRecord> handlers_;
};

} /* namespace jitbuf */

#endif /* INCLUDE_JITBUF_HANDLER_H_ */
