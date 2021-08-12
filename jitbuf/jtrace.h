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

/**
 * Registration / De-registration macros
 * @param jring_ptr: the jring to register to
 * @param msg_name: name of the message. Pass as a token not a string, i.e.,
 *   write my_msg, not "my_msg" as parameter)
 * @param tracepoint_name: name of the tracepoint, without the "tracepoint_"
 *   prefix. Pass as token not as string (i.e., write tcp_recv not "tcp_recv").
 */
#define jtrace_register(_jring, msg_name, tracepoint_name)                                                                     \
  ({                                                                                                                           \
    int __ret;                                                                                                                 \
    /* perform static check for compatibility */                                                                               \
    __jtrace_prototype_##msg_name check_that_prototypes_match = trace_##tracepoint_name;                                       \
    (void)check_that_prototypes_match; /* suppress unused warning */                                                           \
    /* now register */                                                                                                         \
    switch ((_jring)->backend) {                                                                                               \
    case JTRACE_KRING_BACKEND:                                                                                                 \
      __ret = register_trace_##tracepoint_name(__jtrace_kring_probe__##msg_name, (void *)(_jring));                            \
      break;                                                                                                                   \
    case JTRACE_FCP_BACKEND:                                                                                                   \
      __ret = register_trace_##tracepoint_name(__jtrace_fcp_probe__##msg_name, (void *)(_jring));                              \
      break;                                                                                                                   \
    default:                                                                                                                   \
      __ret = -EINVAL;                                                                                                         \
    }                                                                                                                          \
    __ret;                                                                                                                     \
  })

#define jtrace_unregister(_jring, msg_name, tracepoint_name)                                                                   \
  ({                                                                                                                           \
    int __ret;                                                                                                                 \
    switch ((_jring)->backend) {                                                                                               \
    case JTRACE_KRING_BACKEND:                                                                                                 \
      __ret = unregister_trace_##tracepoint_name(__jtrace_kring_probe__##msg_name, (void *)(_jring));                          \
      break;                                                                                                                   \
    case JTRACE_FCP_BACKEND:                                                                                                   \
      __ret = unregister_trace_##tracepoint_name(__jtrace_fcp_probe__##msg_name, (void *)(_jring));                            \
      break;                                                                                                                   \
    default:                                                                                                                   \
      __ret = -EINVAL;                                                                                                         \
    }                                                                                                                          \
    __ret;                                                                                                                     \
  })

/**
 * Registration / de-registration macros with error prints
 */
#define jtrace_register_err(jring_ptr, msg_name, tracepoint_name)                                                              \
  ({                                                                                                                           \
    int __ret = jtrace_register(jring_ptr, msg_name, tracepoint_name);                                                         \
    if (__ret != 0)                                                                                                            \
      pr_err("%s: jtrace could not register msg " #msg_name " to tracepoint " #tracepoint_name " ret=%d\n", __func__, __ret);  \
    __ret;                                                                                                                     \
  })

#define jtrace_unregister_notice(jring_ptr, msg_name, tracepoint_name)                                                         \
  ({                                                                                                                           \
    int __ret = jtrace_unregister(jring_ptr, msg_name, tracepoint_name);                                                       \
    if (__ret != 0)                                                                                                            \
      pr_notice(                                                                                                               \
          "%s: jtrace could not unregister msg " #msg_name " from tracepoint " #tracepoint_name " ret=%d\n", __func__, __ret); \
    __ret;                                                                                                                     \
  })
