// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <platform/types.h>

// from proc(5) - /proc/[pid]/status
#define PROC_STATUS_VIEW_IMPL(X)                                                                                               \
  X(views::NumberView<u64>, voluntary_ctxt_switches, "voluntary_ctxt_switches")                                                \
  X(views::NumberView<u64>, nonvoluntary_ctxt_switches, "nonvoluntary_ctxt_switches")

/* Excluding these fields from the view since we don't care about those
 * This speeds up parsing

  X(std::string_view, Umask, "Umask") \
  X(std::string_view, State, "State") \
  X(std::string_view, Tgid, "Tgid") \
  X(std::string_view, Ngid, "Ngid") \
  X(std::string_view, Pid, "Pid") \
  X(std::string_view, PPid, "PPid") \
  X(std::string_view, TracerPid, "TracerPid") \
  X(std::string_view, Uid, "Uid") \
  X(std::string_view, Gid, "Gid") \
  X(std::string_view, FDSize, "FDSize") \
  X(std::string_view, Groups, "Groups") \
  X(std::string_view, NStgid, "NStgid") \
  X(std::string_view, NSpid, "NSpid") \
  X(std::string_view, NSpgid, "NSpgid") \
  X(std::string_view, NSsid, "NSsid") \
  X(std::string_view, VmPeak, "VmPeak") \
  X(std::string_view, VmSize, "VmSize") \
  X(std::string_view, VmLck, "VmLck") \
  X(std::string_view, VmPin, "VmPin") \
  X(std::string_view, VmHWM, "VmHWM") \
  X(std::string_view, VmRSS, "VmRSS") \
  X(std::string_view, RssAnon, "RssAnon") \
  X(std::string_view, RssFile, "RssFile") \
  X(std::string_view, RssShmem, "RssShmem") \
  X(std::string_view, VmData, "VmData") \
  X(std::string_view, VmStk, "VmStk") \
  X(std::string_view, VmExe, "VmExe") \
  X(std::string_view, VmLib, "VmLib") \
  X(std::string_view, VmPTE, "VmPTE") \
  X(std::string_view, VmSwap, "VmSwap") \
  X(std::string_view, HugetlbPages, "HugetlbPages") \
  X(std::string_view, CoreDumping, "CoreDumping") \
  X(std::string_view, THP_enabled, "THP_enabled") \
  X(std::string_view, Threads, "Threads") \
  X(std::string_view, SigQ, "SigQ") \
  X(std::string_view, SigPnd, "SigPnd") \
  X(std::string_view, ShdPnd, "ShdPnd") \
  X(std::string_view, SigBlk, "SigBlk") \
  X(std::string_view, SigIgn, "SigIgn") \
  X(std::string_view, SigCgt, "SigCgt") \
  X(std::string_view, CapInh, "CapInh") \
  X(std::string_view, CapPrm, "CapPrm") \
  X(std::string_view, CapEff, "CapEff") \
  X(std::string_view, CapBnd, "CapBnd") \
  X(std::string_view, CapAmb, "CapAmb") \
  X(std::string_view, NoNewPrivs, "NoNewPrivs") \
  X(std::string_view, Seccomp, "Seccomp") \
  X(std::string_view, Speculation_Store_Bypass, "Speculation_Store_Bypass") \
  X(std::string_view, Cpus_allowed, "Cpus_allowed") \
  X(std::string_view, Cpus_allowed_list, "Cpus_allowed_list") \
  X(std::string_view, Mems_allowed, "Mems_allowed") \
  X(std::string_view, Mems_allowed_list, "Mems_allowed_list") \
*/
