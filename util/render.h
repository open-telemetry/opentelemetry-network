/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

template <typename T> using DispatchProtocolMemberHandlerFunc = void (T::*)(u64, char *);

// wraps a member function of T as a protocol handler
template <typename T, DispatchProtocolMemberHandlerFunc<T> member_handler_func>
void dispatch_protocol_member_handler(void *context, u64 timestamp, char *buffer)
{
  auto const object = reinterpret_cast<T *>(context);
  (object->*member_handler_func)(timestamp, buffer);
}
