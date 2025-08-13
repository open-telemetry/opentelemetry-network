/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

//
// bpf_debug.h - BPF Debugging Tools
//

#pragma once

#include "tcp-processor/bpf_types.h"

#if DEBUG_LOG
#define DEBUG_PRINTK bpf_trace_printk
#else
#define DEBUG_PRINTK(...)
#endif

static void s_print_bpf_assert(int cond, const char *condstr)
{
  DEBUG_PRINTK("BPF_ASSERT failed: %s\n", condstr);
}

#define BPF_ASSERT_RET(COND, RET)                                                                                              \
  if (!(COND)) {                                                                                                               \
    char __condstr[] = #COND;                                                                                                  \
    s_print_bpf_assert((COND), __condstr);                                                                                     \
    return (RET);                                                                                                              \
  }

#if DEBUG_ENABLE_STACKTRACE

#include <linux/sched.h>
#include <uapi/linux/ptrace.h>

BPF_STACK_TRACE(stack_traces, TABLE_SIZE__STACK_TRACES);

static __always_inline void stack_trace(struct pt_regs *ctx)
{
  char comm[16];
  u64 now = get_timestamp();
  s32 kernel_stack_id = stack_traces.get_stackid(ctx, 0);
  s32 user_stack_id = stack_traces.get_stackid(ctx, BPF_F_USER_STACK);
  u32 tgid = bpf_get_current_pid_tgid() >> 32;
  bpf_get_current_comm(comm, sizeof(comm));

  perf_submit_agent_internal__stack_trace(ctx, now, kernel_stack_id, user_stack_id, tgid, comm);
}

#endif

static __always_inline int __check_broken_in6_addr(struct in6_addr *addr, int line)
{
  if (addr->in6_u.u6_addr16[0] == 0x1140 && addr->in6_u.u6_addr16[2] == 0x007f) {
    bpf_trace_printk("broken in6_addr on line %d\n", line);
    return 1;
  }
  return 0;
}
