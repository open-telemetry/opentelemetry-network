/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

typedef u64 PID_TGID;
typedef u32 TGID;
typedef u32 PID;

// tgid is upper 32 bits of pid_tgid, lower 32 bits is pid
#define TGID_FROM_PID_TGID(X) ((TGID)((X) >> 32))
#define PID_FROM_PID_TGID(X) ((PID)((X)))
// common operation, use this macro to define _pid and _tgid for kprobes
#define GET_PID_TGID                                                                                                           \
  PID_TGID _pid_tgid = bpf_get_current_pid_tgid();                                                                             \
  _pid_tgid &= 0xFFFFFFFFFFFFFFFF;                                                                                             \
  TGID _tgid = TGID_FROM_PID_TGID(_pid_tgid);                                                                                  \
  PID _pid = PID_FROM_PID_TGID(_pid_tgid);                                                                                     \
  if (_tgid == 0 || _pid == 0) {                                                                                               \
    bpf_log(ctx, BPF_LOG_INVALID_PID_TGID, _pid_tgid, 0, 0);                                                                   \
  }                                                                                                                            \
  u64 _cpu = bpf_get_smp_processor_id();

typedef u64 TIMESTAMP;

//
// probe->retprobe argument deferral
// this stuff is a cut-and-paste-error reduction mechanism
// because we often need to pass arguments received in a 'kprobe'
// into its corresponding 'kretprobe', where they are no longer available.
//

#define SAVED_ARGS_TABLE_KEY _pid_tgid
// #define SAVED_ARGS_TABLE_KEY _cpu

#define SAVED_ARGS_TABLE(FUNC) FUNC##_active
#define SAVED_ARGS_TYPE(FUNC) struct FUNC##_active_args_t

#define BEGIN_DECLARE_SAVED_ARGS(FUNC)                                                                                         \
  SAVED_ARGS_TYPE(FUNC)                                                                                                        \
  {

#define END_DECLARE_SAVED_ARGS(FUNC)                                                                                           \
  }                                                                                                                            \
  ;                                                                                                                            \
                                                                                                                               \
  struct {                                                                                                                     \
    __uint(type, BPF_MAP_TYPE_HASH);                                                                                           \
    __type(key, u64);                                                                                                          \
    __type(value, SAVED_ARGS_TYPE(FUNC));                                                                                      \
    __uint(max_entries, 1024);                                                                                                 \
  } SAVED_ARGS_TABLE(FUNC) SEC(".maps");

#define BEGIN_SAVE_ARGS(FUNC)                                                                                                  \
  {                                                                                                                            \
    SAVED_ARGS_TYPE(FUNC) __saved_args = {

#define SAVE_ARG(ARG) .ARG = ARG,

// don't bpf_trace_printk if we have debugging turned off
#if DEBUG_OTHER_MAP_ERRORS
#define __DEBUG_OTHER_MAP_ERRORS_PRINTK false
#else
#define __DEBUG_OTHER_MAP_ERRORS_PRINTK true
#endif

#define END_SAVE_ARGS(FUNC)                                                                                                    \
  }                                                                                                                            \
  ;                                                                                                                            \
  int __ret = bpf_map_update_elem(&SAVED_ARGS_TABLE(FUNC), &SAVED_ARGS_TABLE_KEY, &__saved_args, BPF_NOEXIST);                 \
  if (__ret != 0 && __DEBUG_OTHER_MAP_ERRORS_PRINTK) {                                                                         \
    /* Check if key already exists to distinguish duplicate vs table full */                                                   \
    void *existing = bpf_map_lookup_elem(&SAVED_ARGS_TABLE(FUNC), &SAVED_ARGS_TABLE_KEY);                                      \
    unsigned long long print_tgid = _pid_tgid & 0xFFFFFFFFFFFFFFFF;
    if (existing) {                                                                                                            \
      bpf_trace_printk(#FUNC ": duplicate arg insert. pid_tgid=%ull\n", print_tgid);                                            \
    } else {                                                                                                                   \
      bpf_trace_printk(#FUNC ": args table is full, dropped insert. pid_tgid=%ull\n", print_tgid);                             \
    }                                                                                                                          \
  }                                                                                                                            \
  }

#define GET_ARGS(FUNC, NAME)                                                                                                   \
  SAVED_ARGS_TYPE(FUNC) *NAME = bpf_map_lookup_elem(&SAVED_ARGS_TABLE(FUNC), &SAVED_ARGS_TABLE_KEY);                           \
  if (NAME == NULL) {                                                                                                          \
    __DEBUG_OTHER_MAP_ERRORS_PRINTK || bpf_trace_printk(#FUNC ": args table missing key. tgid=%u\n", _tgid);                   \
  }

#define GET_ARGS_MISSING_OK(FUNC, NAME)                                                                                        \
  SAVED_ARGS_TYPE(FUNC) *NAME = bpf_map_lookup_elem(&SAVED_ARGS_TABLE(FUNC), &SAVED_ARGS_TABLE_KEY);

#define DELETE_ARGS(FUNC) bpf_map_delete_elem(&SAVED_ARGS_TABLE(FUNC), &SAVED_ARGS_TABLE_KEY);

// Timestamp abstraction
// We do this so that eventually we could test the BPF outside
// of the agent (in python or whatever). could replace this to simulate tight
// timing tests eventually too

static TIMESTAMP get_timestamp(void)
{
#ifdef STANDALONE_TCP_PROCESSOR
  return (TIMESTAMP)bpf_ktime_get_ns();
#else
  // Get time, adjust for boot
  return bpf_ktime_get_ns() + boot_time_adjustment;
#endif
}

// Error reporting abstraction
struct BPF_LOG_GLOBALS {
  u32 period_start_ms;
  u32 count;
};
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, u32);
  __type(value, struct BPF_LOG_GLOBALS);
  __uint(max_entries, BPF_MAX_CPUS);
} bpf_log_globals_per_cpu SEC(".maps");

struct bpf_log_args {
  enum BPF_LOG_CODE code;
  u64 arg0;
  u64 arg1;
  u64 arg2;
};

#define bpf_log(ctx, _code, _arg0, _arg1, _arg2)                                                                               \
  do {                                                                                                                         \
    struct bpf_log_args __log_args = {.code = (_code), .arg0 = (_arg0), .arg1 = (_arg1), .arg2 = (_arg2)};                     \
    _bpf_log(__LINE__, ctx, &__log_args);                                                                                      \
  } while (0)

static void _bpf_log(int filelineid, struct pt_regs *ctx, struct bpf_log_args *args)
{

  // Get timestamp in milliseconds
  TIMESTAMP now = get_timestamp();
  u32 now_ms = (u32)(now / 1000000ull);

  // Get per-cpu globals
  int cpu = bpf_get_smp_processor_id();
  if (cpu < 0 || cpu >= BPF_MAX_CPUS) {
    // what to do when 'log' itself fails?
    return;
  }
  struct BPF_LOG_GLOBALS *globals = bpf_map_lookup_elem(&bpf_log_globals_per_cpu, &cpu);
  if (globals == NULL) {
    // what to do when 'log' itself fails?
    return;
  }

  // Have we exceeded the previous period?
  // if so, start a new period
  if (now_ms >= globals->period_start_ms + BPF_LOG_THROTTLE_PERIOD_LENGTH_MS) {
    globals->period_start_ms = now_ms;
    globals->count = 0;
  }

  // Increment the count for this period
  globals->count += 1;

  // If the count has exceeded the max per period, log if it's the first one, else drop
  if (globals->count == (BPF_LOG_THROTTLE_MAX_PER_PERIOD + 1)) {
#ifdef STANDALONE_TCP_PROCESSOR
    bpf_trace_printk("bpf_log: throttled on cpu %u\n", cpu);
#else
    perf_submit_agent_internal__bpf_log(ctx, now, (u64)filelineid, (u64)BPF_LOG_THROTTLED, cpu, 0, 0);
#endif
  } else if (globals->count <= BPF_LOG_THROTTLE_MAX_PER_PERIOD) {
#ifdef STANDALONE_TCP_PROCESSOR
    bpf_trace_printk("bpf_log: filelineid=%d, code=%u, now=%llu\n", filelineid, args->code, now);
    bpf_trace_printk("         args=%llu, %llu, %llu\n", args->arg0, args->arg1, args->arg2);
#else
    perf_submit_agent_internal__bpf_log(ctx, now, (u64)filelineid, (u64)args->code, args->arg0, args->arg1, args->arg2);
#endif
  }
}

// Useful conversion from ip4 space to ip6 space for addresses
inline static struct in6_addr make_ipv6_address(__be32 addr)
{
  struct in6_addr addr6 = {.in6_u.u6_addr32 = {0, 0, 0xffff0000, addr}};
  return addr6;
}
