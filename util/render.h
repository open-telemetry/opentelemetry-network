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

#pragma once

template <typename T> using DispatchProtocolMemberHandlerFunc = void (T::*)(u64, char *);

// wraps a member function of T as a protocol handler
template <typename T, DispatchProtocolMemberHandlerFunc<T> member_handler_func>
void dispatch_protocol_member_handler(void *context, u64 timestamp, char *buffer)
{
  auto const object = reinterpret_cast<T *>(context);
  (object->*member_handler_func)(timestamp, buffer);
}
