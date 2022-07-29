// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <util/proc_ops.h>

TEST(proc_stat_view, pid_3062214)
{
  constexpr std::string_view data =
      R"(3062214 (multi thread) S 2829421 3062214 2829421 34819 3062214 1077936128 138 0 0 0 3 0 0 0 20 0 4 0 183646228 29917184 289 18446744073709551615 94693039165440 94693039850013 140729897079600 0 0 0 0 0 0 0 0 0 17 2 0 0 0 0 0 94693040067224 94693040096472 94693057708032 140729897082593 140729897082611 140729897082611 140729897086950 0)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(3062214, view.pid);
  EXPECT_EQ("multi thread", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(2829421, view.ppid);
  EXPECT_EQ(3062214, view.pgrp);
  EXPECT_EQ(2829421, view.session);
  EXPECT_EQ(34819, view.tty_nr);
  EXPECT_EQ(3062214, view.tpgid);
  EXPECT_EQ(1077936128, view.flags);
  EXPECT_EQ(138, view.minflt);
  EXPECT_EQ(0, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(3, view.utime);
  EXPECT_EQ(0, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(0, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(4, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(183646228ull, view.starttime);
  EXPECT_EQ(29917184, view.vsize);
  EXPECT_EQ(289, view.rss);
  EXPECT_EQ(18446744073709551615ul, view.rsslim);
  EXPECT_EQ(94693039165440ul, view.startcode);
  EXPECT_EQ(94693039850013, view.endcode);
  EXPECT_EQ(140729897079600, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(0, view.blocked);
  EXPECT_EQ(0, view.sigignore);
  EXPECT_EQ(0, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(2, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(94693040067224, view.start_data);
  EXPECT_EQ(94693040096472, view.end_data);
  EXPECT_EQ(94693057708032, view.start_brk);
  EXPECT_EQ(140729897082593, view.arg_start);
  EXPECT_EQ(140729897082611, view.arg_end);
  EXPECT_EQ(140729897082611, view.env_start);
  EXPECT_EQ(140729897086950, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, pid_3049866)
{
  constexpr std::string_view data =
      R"(3049866 (multi-thread) S 2829421 3049866 2829421 34819 3049866 1077936128 136 0 0 0 22 26 0 0 20 0 4 0 183619102 29917184 305 18446744073709551615 94892149485568 94892150170141 140724065920688 0 0 0 0 0 0 0 0 0 17 1 0 0 0 0 0 94892150387352 94892150416600 94892153749504 140724065922713 140724065922755 140724065922755 140724065927118 0)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(3049866, view.pid);
  EXPECT_EQ("multi-thread", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(2829421, view.ppid);
  EXPECT_EQ(3049866, view.pgrp);
  EXPECT_EQ(2829421, view.session);
  EXPECT_EQ(34819, view.tty_nr);
  EXPECT_EQ(3049866, view.tpgid);
  EXPECT_EQ(1077936128, view.flags);
  EXPECT_EQ(136, view.minflt);
  EXPECT_EQ(0, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(22, view.utime);
  EXPECT_EQ(26, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(0, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(4, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(183619102ull, view.starttime);
  EXPECT_EQ(29917184, view.vsize);
  EXPECT_EQ(305, view.rss);
  EXPECT_EQ(18446744073709551615ul, view.rsslim);
  EXPECT_EQ(94892149485568ul, view.startcode);
  EXPECT_EQ(94892150170141ul, view.endcode);
  EXPECT_EQ(140724065920688ul, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(0, view.blocked);
  EXPECT_EQ(0, view.sigignore);
  EXPECT_EQ(0, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(1, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(94892150387352, view.start_data);
  EXPECT_EQ(94892150416600, view.end_data);
  EXPECT_EQ(94892153749504, view.start_brk);
  EXPECT_EQ(140724065922713, view.arg_start);
  EXPECT_EQ(140724065922755, view.arg_end);
  EXPECT_EQ(140724065922755, view.env_start);
  EXPECT_EQ(140724065927118, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, pid_3070715)
{
  constexpr std::string_view data =
      R"(3070715 (multi (99) thre) S 2829421 3070715 2829421 34819 3070715 1077936128 139 0 0 0 2 1 0 0 20 0 4 0 183650343 29917184 306 18446744073709551615 94786539032576 94786539717149 140728695372256 0 0 0 0 0 0 0 0 0 17 0 0 0 0 0 0 94786539934360 94786539963608 94786569641984 140728695377618 140728695377641 140728695377641 140728695381985 0)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(3070715, view.pid);
  EXPECT_EQ("multi (99) thre", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(2829421, view.ppid);
  EXPECT_EQ(3070715, view.pgrp);
  EXPECT_EQ(2829421, view.session);
  EXPECT_EQ(34819, view.tty_nr);
  EXPECT_EQ(3070715, view.tpgid);
  EXPECT_EQ(1077936128, view.flags);
  EXPECT_EQ(139, view.minflt);
  EXPECT_EQ(0, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(2, view.utime);
  EXPECT_EQ(1, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(0, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(4, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(183650343ull, view.starttime);
  EXPECT_EQ(29917184, view.vsize);
  EXPECT_EQ(306, view.rss);
  EXPECT_EQ(18446744073709551615ul, view.rsslim);
  EXPECT_EQ(94786539032576ul, view.startcode);
  EXPECT_EQ(94786539717149ul, view.endcode);
  EXPECT_EQ(140728695372256ul, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(0, view.blocked);
  EXPECT_EQ(0, view.sigignore);
  EXPECT_EQ(0, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(0, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(94786539934360, view.start_data);
  EXPECT_EQ(94786539963608, view.end_data);
  EXPECT_EQ(94786569641984, view.start_brk);
  EXPECT_EQ(140728695377618, view.arg_start);
  EXPECT_EQ(140728695377641, view.arg_end);
  EXPECT_EQ(140728695377641, view.env_start);
  EXPECT_EQ(140728695381985, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_io_view, pid_6545)
{
  constexpr std::string_view data = R"(rchar: 783422713330
wchar: 197059793079
syscr: 127142356
syscw: 71858467
read_bytes: 3966263296
write_bytes: 75290411008
cancelled_write_bytes: 11436556288)";

  ProcIoView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(783422713330ull, view.rchar);
  EXPECT_EQ(197059793079ull, view.wchar);
  EXPECT_EQ(127142356ull, view.syscr);
  EXPECT_EQ(71858467ull, view.syscw);
  EXPECT_EQ(3966263296ull, view.read_bytes);
  EXPECT_EQ(75290411008ull, view.write_bytes);
  EXPECT_EQ(11436556288ull, view.cancelled_write_bytes);
}

TEST(proc_io_view, pid_662910)
{
  constexpr std::string_view data = R"(rchar: 9069720
wchar: 337393
syscr: 1131
syscw: 76
read_bytes: 27787264
write_bytes: 2957312
cancelled_write_bytes: 16384)";

  ProcIoView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(9069720ull, view.rchar);
  EXPECT_EQ(337393ull, view.wchar);
  EXPECT_EQ(1131ull, view.syscr);
  EXPECT_EQ(76ull, view.syscw);
  EXPECT_EQ(27787264ull, view.read_bytes);
  EXPECT_EQ(2957312ull, view.write_bytes);
  EXPECT_EQ(16384ull, view.cancelled_write_bytes);
}

TEST(proc_io_view, pid_717621)
{
  constexpr std::string_view data = R"(rchar: 41578
wchar: 8
syscr: 53
syscw: 2
read_bytes: 0
write_bytes: 4096
cancelled_write_bytes: 0)";

  ProcIoView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(41578ull, view.rchar);
  EXPECT_EQ(8ull, view.wchar);
  EXPECT_EQ(53ull, view.syscr);
  EXPECT_EQ(2ull, view.syscw);
  EXPECT_EQ(0ull, view.read_bytes);
  EXPECT_EQ(4096ull, view.write_bytes);
  EXPECT_EQ(0ull, view.cancelled_write_bytes);
}

TEST(proc_io_view, pid_98303)
{
  constexpr std::string_view data = R"(rchar: 394415079
wchar: 82744660
syscr: 554996
syscw: 371456
read_bytes: 912367616
write_bytes: 27467776
cancelled_write_bytes: 610304)";

  ProcIoView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(394415079ull, view.rchar);
  EXPECT_EQ(82744660ull, view.wchar);
  EXPECT_EQ(554996ull, view.syscr);
  EXPECT_EQ(371456ull, view.syscw);
  EXPECT_EQ(912367616ull, view.read_bytes);
  EXPECT_EQ(27467776ull, view.write_bytes);
  EXPECT_EQ(610304ull, view.cancelled_write_bytes);
}

TEST(proc_io_view, pid_996354)
{
  constexpr std::string_view data = R"(rchar: 42616
wchar: 1236
syscr: 2570
syscw: 1236
read_bytes: 0
write_bytes: 0
cancelled_write_bytes: 0)";

  ProcIoView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(42616ull, view.rchar);
  EXPECT_EQ(1236ull, view.wchar);
  EXPECT_EQ(2570ull, view.syscr);
  EXPECT_EQ(1236ull, view.syscw);
  EXPECT_EQ(0ull, view.read_bytes);
  EXPECT_EQ(0ull, view.write_bytes);
  EXPECT_EQ(0ull, view.cancelled_write_bytes);
}

TEST(proc_status_view, pid_6545)
{
  constexpr std::string_view data = R"(Name:	docker-containe
Umask:	0022
State:	S (sleeping)
Tgid:	6545
Ngid:	0
Pid:	6545
PPid:	6486
TracerPid:	0
Uid:	0	0	0	0
Gid:	0	0	0	0
FDSize:	64
Groups:	 
NStgid:	6545
NSpid:	6545
NSpgid:	6545
NSsid:	6545
VmPeak:	 1760208 kB
VmSize:	 1694672 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	   42008 kB
VmRSS:	   23800 kB
RssAnon:	   12708 kB
RssFile:	   11092 kB
RssShmem:	       0 kB
VmData:	  279540 kB
VmStk:	     132 kB
VmExe:	   16636 kB
VmLib:	       0 kB
VmPTE:	     340 kB
VmSwap:	    3080 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	1
Threads:	22
SigQ:	0/126119
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	fffffffe3bfa2800
SigIgn:	0000000000000000
SigCgt:	ffffffffffc1feff
CapInh:	0000000000000000
CapPrm:	0000003fffffffff
CapEff:	0000003fffffffff
CapBnd:	0000003fffffffff
CapAmb:	0000000000000000
NoNewPrivs:	0
Seccomp:	0
Speculation_Store_Bypass:	thread vulnerable
Cpus_allowed:	3f
Cpus_allowed_list:	0-5
Mems_allowed:	00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	290
nonvoluntary_ctxt_switches:	0)";

  ProcStatusView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(290, view.voluntary_ctxt_switches);
  EXPECT_EQ(0, view.nonvoluntary_ctxt_switches);
}

TEST(proc_status_view, pid_662910)
{
  constexpr std::string_view data = R"(Name:	mysqld
Umask:	0006
State:	S (sleeping)
Tgid:	662910
Ngid:	0
Pid:	662910
PPid:	1
TracerPid:	0
Uid:	113	113	113	113
Gid:	120	120	120	120
FDSize:	128
Groups:	120 
NStgid:	662910
NSpid:	662910
NSpgid:	662910
NSsid:	662910
VmPeak:	 1972020 kB
VmSize:	 1906484 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	   81016 kB
VmRSS:	   57224 kB
RssAnon:	   47328 kB
RssFile:	    9896 kB
RssShmem:	       0 kB
VmData:	  701244 kB
VmStk:	     132 kB
VmExe:	   13312 kB
VmLib:	     424 kB
VmPTE:	     432 kB
VmSwap:	   19224 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	1
Threads:	30
SigQ:	0/126119
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000087007
SigIgn:	0000000000001000
SigCgt:	00000001800066e9
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	0000000000004000
CapAmb:	0000000000000000
NoNewPrivs:	1
Seccomp:	2
Speculation_Store_Bypass:	thread force mitigated
Cpus_allowed:	3f
Cpus_allowed_list:	0-5
Mems_allowed:	00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	233
nonvoluntary_ctxt_switches:	18)";

  ProcStatusView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(233, view.voluntary_ctxt_switches);
  EXPECT_EQ(18, view.nonvoluntary_ctxt_switches);
}

TEST(proc_status_view, pid_717621)
{
  constexpr std::string_view data = R"(Name:	VBoxXPCOMIPCD
Umask:	0077
State:	S (sleeping)
Tgid:	717621
Ngid:	0
Pid:	717621
PPid:	1
TracerPid:	0
Uid:	1000	1000	1000	1000
Gid:	1000	1000	1000	1000
FDSize:	64
Groups:	24 25 27 29 30 44 46 109 116 1000 
NStgid:	717621
NSpid:	717621
NSpgid:	717621
NSsid:	717620
VmPeak:	   67896 kB
VmSize:	   67688 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	   13744 kB
VmRSS:	   13568 kB
RssAnon:	    2780 kB
RssFile:	   10788 kB
RssShmem:	       0 kB
VmData:	   10384 kB
VmStk:	     136 kB
VmExe:	      20 kB
VmLib:	   14796 kB
VmPTE:	     108 kB
VmSwap:	       0 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	0
Threads:	1
SigQ:	2/126119
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000002000
SigIgn:	0000000000001002
SigCgt:	1000000180010000
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	0000003fffffffff
CapAmb:	0000000000000000
NoNewPrivs:	0
Seccomp:	0
Speculation_Store_Bypass:	thread vulnerable
Cpus_allowed:	3f
Cpus_allowed_list:	0-5
Mems_allowed:	00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	929318
nonvoluntary_ctxt_switches:	6003)";

  ProcStatusView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(929318, view.voluntary_ctxt_switches);
  EXPECT_EQ(6003, view.nonvoluntary_ctxt_switches);
}

TEST(proc_status_view, pid_98303)
{
  constexpr std::string_view data = R"(Name:	bash
Umask:	0022
State:	S (sleeping)
Tgid:	98303
Ngid:	0
Pid:	98303
PPid:	13384
TracerPid:	0
Uid:	1000	1000	1000	1000
Gid:	1000	1000	1000	1000
FDSize:	256
Groups:	24 25 27 29 30 44 46 109 116 1000 
NStgid:	98303
NSpid:	98303
NSpgid:	98303
NSsid:	98303
VmPeak:	    9600 kB
VmSize:	    9564 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	    6320 kB
VmRSS:	    5524 kB
RssAnon:	    2712 kB
RssFile:	    2812 kB
RssShmem:	       0 kB
VmData:	    2780 kB
VmStk:	     132 kB
VmExe:	     880 kB
VmLib:	    1560 kB
VmPTE:	      52 kB
VmSwap:	       0 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	1
Threads:	1
SigQ:	2/126119
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000000000
SigIgn:	0000000000380004
SigCgt:	00000001cb817efb
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	0000003fffffffff
CapAmb:	0000000000000000
NoNewPrivs:	0
Seccomp:	0
Speculation_Store_Bypass:	thread vulnerable
Cpus_allowed:	3f
Cpus_allowed_list:	0-5
Mems_allowed:	00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	4562
nonvoluntary_ctxt_switches:	61)";

  ProcStatusView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(4562, view.voluntary_ctxt_switches);
  EXPECT_EQ(61, view.nonvoluntary_ctxt_switches);
}

TEST(proc_status_view, pid_996354)
{
  constexpr std::string_view data = R"(Name:	chrome
Umask:	0022
State:	S (sleeping)
Tgid:	996354
Ngid:	0
Pid:	996354
PPid:	7077
TracerPid:	0
Uid:	1000	1000	1000	1000
Gid:	1000	1000	1000	1000
FDSize:	128
Groups:	24 25 27 29 30 44 46 109 116 1000 
NStgid:	996354	117016
NSpid:	996354	117016
NSpgid:	7061	0
NSsid:	7061	0
VmPeak:	 8714700 kB
VmSize:	 4712640 kB
VmLck:	       0 kB
VmPin:	       0 kB
VmHWM:	  189412 kB
VmRSS:	  161136 kB
RssAnon:	   69048 kB
RssFile:	   82976 kB
RssShmem:	    9112 kB
VmData:	  253240 kB
VmStk:	     132 kB
VmExe:	  144868 kB
VmLib:	   18104 kB
VmPTE:	    1140 kB
VmSwap:	       0 kB
HugetlbPages:	       0 kB
CoreDumping:	0
THP_enabled:	1
Threads:	12
SigQ:	2/126119
SigPnd:	0000000000000000
ShdPnd:	0000000000000000
SigBlk:	0000000000000000
SigIgn:	0000000000001002
SigCgt:	00000001c0010000
CapInh:	0000000000000000
CapPrm:	0000000000000000
CapEff:	0000000000000000
CapBnd:	0000003fffffffff
CapAmb:	0000000000000000
NoNewPrivs:	1
Seccomp:	2
Speculation_Store_Bypass:	thread force mitigated
Cpus_allowed:	3f
Cpus_allowed_list:	0-5
Mems_allowed:	00000000,00000001
Mems_allowed_list:	0
voluntary_ctxt_switches:	2792
nonvoluntary_ctxt_switches:	281)";

  ProcStatusView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(2792, view.voluntary_ctxt_switches);
  EXPECT_EQ(281, view.nonvoluntary_ctxt_switches);
}

TEST(proc_stat_view, pid_19929)
{
  constexpr std::string_view data =
      R"(19929 (docker-containe) S 2075 19929 2075 0 -1 1077936384 2186078 5647 0 0 6716 10474 0 3 20 0 10 0 164204224 7675904 1075 18446744073709551615 4194304 6343957 140724213514144 0 0 0 1006249984 0 2143420159 0 0 0 17 1 0 0 0 0 0 8261632 8367936 36073472 140724213521731 140724213522062 140724213522062 140724213522392 0)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(19929, view.pid);
  EXPECT_EQ("docker-containe", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(2075, view.ppid);
  EXPECT_EQ(19929, view.pgrp);
  EXPECT_EQ(2075, view.session);
  EXPECT_EQ(0, view.tty_nr);
  EXPECT_EQ(-1, view.tpgid);
  EXPECT_EQ(1077936384, view.flags);
  EXPECT_EQ(2186078, view.minflt);
  EXPECT_EQ(5647, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(6716, view.utime);
  EXPECT_EQ(10474, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(3, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(10, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(164204224ull, view.starttime);
  EXPECT_EQ(7675904, view.vsize);
  EXPECT_EQ(1075, view.rss);
  EXPECT_EQ(18446744073709551615ul, view.rsslim);
  EXPECT_EQ(4194304ul, view.startcode);
  EXPECT_EQ(6343957ul, view.endcode);
  EXPECT_EQ(140724213514144, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(1006249984, view.blocked);
  EXPECT_EQ(0, view.sigignore);
  EXPECT_EQ(2143420159, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(1, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(8261632, view.start_data);
  EXPECT_EQ(8367936, view.end_data);
  EXPECT_EQ(36073472, view.start_brk);
  EXPECT_EQ(140724213521731, view.arg_start);
  EXPECT_EQ(140724213522062, view.arg_end);
  EXPECT_EQ(140724213522062, view.env_start);
  EXPECT_EQ(140724213522392, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, ipv6_addrconf)
{
  constexpr std::string_view data =
      R"(80 (ipv6_addrconf) I 2 0 0 0 -1 69238880 0 0 0 0 0 0 0 0 0 -20 1 0 87 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 1 0 0 0 0 0 0 0 0 0 0 0 0 0
)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(80, view.pid);
  EXPECT_EQ("ipv6_addrconf", view.comm);
  EXPECT_TRUE(ProcessState::unknown == sanitize_enum(view.state));
  EXPECT_EQ(2, view.ppid);
  EXPECT_EQ(0, view.pgrp);
  EXPECT_EQ(0, view.session);
  EXPECT_EQ(0, view.tty_nr);
  EXPECT_EQ(-1, view.tpgid);
  EXPECT_EQ(69238880, view.flags);
  EXPECT_EQ(0, view.minflt);
  EXPECT_EQ(0, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(0, view.utime);
  EXPECT_EQ(0, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(0, view.cstime);
  EXPECT_EQ(0, view.priority);
  EXPECT_EQ(-20, view.nice);
  EXPECT_EQ(1, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(87ull, view.starttime);
  EXPECT_EQ(0, view.vsize);
  EXPECT_EQ(0, view.rss);
  EXPECT_EQ(18446744073709551615ull, view.rsslim);
  EXPECT_EQ(0ul, view.startcode);
  EXPECT_EQ(0ul, view.endcode);
  EXPECT_EQ(0, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(0, view.blocked);
  EXPECT_EQ(2147483647, view.sigignore);
  EXPECT_EQ(0, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(1, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(0, view.start_data);
  EXPECT_EQ(0, view.end_data);
  EXPECT_EQ(0, view.start_brk);
  EXPECT_EQ(0, view.arg_start);
  EXPECT_EQ(0, view.arg_end);
  EXPECT_EQ(0, view.env_start);
  EXPECT_EQ(0, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, kernel_collector)
{
  constexpr std::string_view data =
      R"(39973 (kernel-collecto) S 1293 39973 1293 34817 39973 4194304 393 27583 0 0 0 0 7 8 20 0 1 0 9864441 6959104 906 18446744073709551615 94780814675968 94780815573037 140724866766880 0 0 0 65536 4 65538 0 0 0 17 0 0 0 0 0 0 94780815803376 94780815850756 94780843311104 140724866770313 140724866770428 140724866770428 140724866772962 0
)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(39973, view.pid);
  EXPECT_EQ("kernel-collecto", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(1293, view.ppid);
  EXPECT_EQ(39973, view.pgrp);
  EXPECT_EQ(1293, view.session);
  EXPECT_EQ(34817, view.tty_nr);
  EXPECT_EQ(39973, view.tpgid);
  EXPECT_EQ(4194304, view.flags);
  EXPECT_EQ(393, view.minflt);
  EXPECT_EQ(27583, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(0, view.utime);
  EXPECT_EQ(0, view.stime);
  EXPECT_EQ(7, view.cutime);
  EXPECT_EQ(8, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(1, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(9864441, view.starttime);
  EXPECT_EQ(6959104, view.vsize);
  EXPECT_EQ(906, view.rss);
  EXPECT_EQ(18446744073709551615ull, view.rsslim);
  EXPECT_EQ(94780814675968, view.startcode);
  EXPECT_EQ(94780815573037, view.endcode);
  EXPECT_EQ(140724866766880, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(65536, view.blocked);
  EXPECT_EQ(4, view.sigignore);
  EXPECT_EQ(65538, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(0, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(94780815803376, view.start_data);
  EXPECT_EQ(94780815850756, view.end_data);
  EXPECT_EQ(94780843311104, view.start_brk);
  EXPECT_EQ(140724866770313, view.arg_start);
  EXPECT_EQ(140724866770428, view.arg_end);
  EXPECT_EQ(140724866770428, view.env_start);
  EXPECT_EQ(140724866772962, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, collector_entry)
{
  constexpr std::string_view data =
      R"(40060 (collector-entry) S 40043 40060 40060 34816 40060 4194560 441 81195 0 0 0 0 41 5 20 0 1 0 9864881 3956736 782 18446744073709551615 94886840201216 94886841096077 140732640240592 0 0 0 65536 4 65538 0 0 0 17 0 0 0 0 0 0 94886841324528 94886841371908 94886870679552 140732640246970 140732640247110 140732640247110 140732640247771 0
)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(40060, view.pid);
  EXPECT_EQ("collector-entry", view.comm);
  EXPECT_TRUE(ProcessState::interruptible_sleep == sanitize_enum(view.state));
  EXPECT_EQ(40043, view.ppid);
  EXPECT_EQ(40060, view.pgrp);
  EXPECT_EQ(40060, view.session);
  EXPECT_EQ(34816, view.tty_nr);
  EXPECT_EQ(40060, view.tpgid);
  EXPECT_EQ(4194560, view.flags);
  EXPECT_EQ(441, view.minflt);
  EXPECT_EQ(81195, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(0, view.utime);
  EXPECT_EQ(0, view.stime);
  EXPECT_EQ(41, view.cutime);
  EXPECT_EQ(5, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(1, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(9864881, view.starttime);
  EXPECT_EQ(3956736, view.vsize);
  EXPECT_EQ(782, view.rss);
  EXPECT_EQ(18446744073709551615ull, view.rsslim);
  EXPECT_EQ(94886840201216, view.startcode);
  EXPECT_EQ(94886841096077, view.endcode);
  EXPECT_EQ(140732640240592, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(65536, view.blocked);
  EXPECT_EQ(4, view.sigignore);
  EXPECT_EQ(65538, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(0, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(94886841324528, view.start_data);
  EXPECT_EQ(94886841371908, view.end_data);
  EXPECT_EQ(94886870679552, view.start_brk);
  EXPECT_EQ(140732640246970, view.arg_start);
  EXPECT_EQ(140732640247110, view.arg_end);
  EXPECT_EQ(140732640247110, view.env_start);
  EXPECT_EQ(140732640247771, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}

TEST(proc_stat_view, kworker_39842)
{
  constexpr std::string_view data =
      R"(39842 (kworker/u4:0-events_unbound) I 2 0 0 0 -1 69238880 0 315 0 0 0 0 0 0 20 0 1 0 9863357 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 0 0 0 0 0 0 0 0 0 0 0 0 0 0
)";

  ProcStatView const view(data);
  EXPECT_TRUE(view.valid());

  EXPECT_EQ(39842, view.pid);
  EXPECT_EQ("kworker/u4:0-events_unbound", view.comm);
  EXPECT_TRUE(ProcessState::unknown == sanitize_enum(view.state));
  EXPECT_EQ(2, view.ppid);
  EXPECT_EQ(0, view.pgrp);
  EXPECT_EQ(0, view.session);
  EXPECT_EQ(0, view.tty_nr);
  EXPECT_EQ(-1, view.tpgid);
  EXPECT_EQ(69238880, view.flags);
  EXPECT_EQ(0, view.minflt);
  EXPECT_EQ(315, view.cminflt);
  EXPECT_EQ(0, view.majflt);
  EXPECT_EQ(0, view.cmajflt);
  EXPECT_EQ(0, view.utime);
  EXPECT_EQ(0, view.stime);
  EXPECT_EQ(0, view.cutime);
  EXPECT_EQ(0, view.cstime);
  EXPECT_EQ(20, view.priority);
  EXPECT_EQ(0, view.nice);
  EXPECT_EQ(1, view.num_threads);
  EXPECT_EQ(0, view.itrealvalue);
  EXPECT_EQ(9863357, view.starttime);
  EXPECT_EQ(0, view.vsize);
  EXPECT_EQ(0, view.rss);
  EXPECT_EQ(18446744073709551615ull, view.rsslim);
  EXPECT_EQ(0, view.startcode);
  EXPECT_EQ(0, view.endcode);
  EXPECT_EQ(0, view.startstack);
  EXPECT_EQ(0, view.kstkesp);
  EXPECT_EQ(0, view.kstkeip);
  EXPECT_EQ(0, view.signal);
  EXPECT_EQ(0, view.blocked);
  EXPECT_EQ(2147483647, view.sigignore);
  EXPECT_EQ(0, view.sigcatch);
  EXPECT_EQ(0, view.wchan);
  EXPECT_EQ(0, view.nswap);
  EXPECT_EQ(0, view.cnswap);
  EXPECT_EQ(17, view.exit_signal);
  EXPECT_EQ(0, view.processor);
  EXPECT_EQ(0, view.rt_priority);
  EXPECT_EQ(0, view.policy);
  EXPECT_EQ(0, view.delayacct_blkio_ticks);
  EXPECT_EQ(0, view.guest_time);
  EXPECT_EQ(0, view.cguest_time);
  EXPECT_EQ(0, view.start_data);
  EXPECT_EQ(0, view.end_data);
  EXPECT_EQ(0, view.start_brk);
  EXPECT_EQ(0, view.arg_start);
  EXPECT_EQ(0, view.arg_end);
  EXPECT_EQ(0, view.env_start);
  EXPECT_EQ(0, view.env_end);
  EXPECT_EQ(0, view.exit_code);
}
