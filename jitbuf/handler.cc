// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <jitbuf/handler.h>

#include <jitbuf/transformer.h>
#include <sstream>
#include <string>

jitbuf::Handler::Handler(bool fail_unadded, bool fail_unknown)
    : fail_unadded_(fail_unadded), fail_unknown_(fail_unknown), remote_timestamp_(0)
{}

void jitbuf::Handler::add(
    std::shared_ptr<jitbuf::TransformRecord> &transform, handler_func_t handler_func, std::shared_ptr<Service> service)
{
  struct HandlerRecord record;
  record.transform = *transform;
  record.handler_func = handler_func;
  record.priv = service.get();
  record.transform_ptr = transform;
  record.service_ptr = service;

  auto res = handlers_.insert({transform->msg_rpc_id, record});
  if (res.second != true)
    throw std::runtime_error("rpc_id already exists");
}

void jitbuf::Handler::add(std::shared_ptr<jitbuf::TransformRecord> &transform, handler_func_t handler_func, void *priv)
{
  struct HandlerRecord record;
  record.transform = *transform;
  record.handler_func = handler_func;
  record.priv = priv;
  record.transform_ptr = transform;

  auto res = handlers_.insert({transform->msg_rpc_id, record});
  if (res.second != true)
    throw std::runtime_error("rpc_id already exists");
}

void jitbuf::Handler::add_identity(TransformBuilder &builder, std::shared_ptr<jitbuf::Service> service)
{
  const handler_package &package(service->get_package());
  for (int i = 0; i < package.size; i++) {
    const handler_descriptor *hdesc = &package.descriptors[i];
    const jb_descriptor *jbdesc = hdesc->descriptor;

    /* get a string from the descriptor */
    std::string descriptor_str(jbdesc->buf, jbdesc->size);

    /* add the service */
    auto xform = builder.get_xform(descriptor_str);
    add(xform, hdesc->handler_func, service);
  }
}

void jitbuf::Handler::add_identity(TransformBuilder &builder, jitbuf::Service *service)
{
  const handler_package &package(service->get_package());
  for (int i = 0; i < package.size; i++) {
    const handler_descriptor *hdesc = &package.descriptors[i];
    const jb_descriptor *jbdesc = hdesc->descriptor;

    /* get a string from the descriptor */
    std::string descriptor_str(jbdesc->buf, jbdesc->size);

    /* add the service */
    auto xform = builder.get_xform(descriptor_str);
    add(xform, hdesc->handler_func, service);
  }
}

int jitbuf::Handler::handle(const char *msg, uint32_t len, u64 flags)
{
  int processed = 0;

  /* Handle timestamps */
  if (flags & HFLAG_TIMESTAMPED) {
    if (len < sizeof(u64))
      return -EINVAL;

    remote_timestamp_ = *(u64 *)msg;
    msg += sizeof(u64);
    len -= sizeof(u64);
    processed += sizeof(u64);
  }

  /* get RPC ID */
  if (len < 2)
    return -EINVAL;

  uint16_t rpc_id = *(uint16_t *)msg;

  /* find handler for RPC ID */
  auto iter = handlers_.find(rpc_id);
  if (iter == handlers_.end()) {
    if (fail_unadded_) {
      std::ostringstream oss;
      oss << "message rpc_id=" << rpc_id << " not in handler hash";
      throw std::runtime_error(oss.str());
    } else {
      return -EINVAL;
    }
  }

  /* safety check message size */
  HandlerRecord &record = iter->second;
  if (len < record.transform.size)
    return -EINVAL;

  /* invoke handler */
  uint16_t size = record.handler_func(msg, record.transform.xform, record.priv);

  /* if we didn't get all the dynamic sized part, punt */
  if (size > len)
    return -EINVAL;

  return size + processed;
}

int jitbuf::Handler::handle(const std::string &msg, u64 flags)
{
  return handle(msg.data(), msg.length(), flags);
}

jitbuf::Transform jitbuf::Handler::get_transform(int rpc_id)
{
  return handlers_.at(rpc_id).transform.xform;
}

int jitbuf::Handler::handle_multiple(const char *msg, u64 len, u64 flags)
{
  u64 processed = 0;
  u64 remaining = len;
  int ret = 0;

  while (len > processed) {
    int ret = handle(msg + processed, (remaining > ((u32)-1) ? ((u32)-1) : remaining), flags);
    if (ret < 0)
      break;

    /* sanity check, should not happen */
    if ((ret + processed > len) || ((u64)(ret + processed) < processed))
      throw std::runtime_error("Possible overflow in handle_multiple");

    processed += ret;
    remaining -= ret;
  }

  if (processed > 0)
    return processed;

  /* error, return code */
  return ret;
}

int jitbuf::Handler::handle_multiple(const std::string &msg, u64 flags)
{
  return handle_multiple(msg.data(), msg.length(), flags);
}
