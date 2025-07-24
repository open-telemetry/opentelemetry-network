// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "kernel_symbols.h"

#include <sstream>
#include <stdexcept>
#include <string_view>

#include <gtest/gtest.h>

static constexpr std::string_view EXAMPLE_KALLSYMS = R"delim(
0000000000000000 T startup_64
0000000000000000 T _stext
0000000000000000 T _text
0000000000000000 T secondary_startup_64
0000000000000000 T secondary_startup_64_no_verify
0000000000000000 t verify_cpu
0000000000000000 T sev_verify_cbit
0000000000000000 T start_cpu0
0000000000000000 T __startup_64
0000000000000000 T startup_64_setup_env
0000000000000000 b ignore_oc	[ehci_hcd]
0000000000000000 t iso_stream_find	[ehci_hcd]
0000000000000000 r smask_out.94	[ehci_hcd]
0000000000000000 d __UNIQUE_ID_ddebug215.19 	[ehci_hcd]
0000000000000000 d __UNIQUE_ID_ddebug121.49 	[ehci_hcd]
0000000000000000 t ehci_run.cold	[ehci_hcd]
ffffffffc09a60cc r __ksymtab_nf_conntrack_alter_reply	[nf_conntrack]
ffffffffc09aa74e r __kstrtab_nf_conntrack_alter_reply	[nf_conntrack]
ffffffffc0996bc0 t nf_conntrack_alter_reply	[nf_conntrack]
ffffffffc09cdbf0 t ctnetlink_dump_tuples_proto	[nf_conntrack_netlink]
ffffffffc09ce230 t ctnetlink_dump_tuples_ip	[nf_conntrack_netlink]
)delim";

static constexpr std::string_view UNKNOWN_SYMBOL = "DEFINITELY_NOT_A_KERNEL_SYMBOL";

TEST(KernelSymbolsTest, ReadStream)
{
  std::stringstream stream(EXAMPLE_KALLSYMS.data());

  auto ks = read_proc_kallsyms(stream);

  EXPECT_TRUE(ks.contains("verify_cpu"));
  EXPECT_TRUE(ks.contains("nf_conntrack_alter_reply"));
  EXPECT_TRUE(ks.contains("ctnetlink_dump_tuples_ip"));
  EXPECT_TRUE(ks.contains("iso_stream_find"));

  EXPECT_FALSE(ks.contains(UNKNOWN_SYMBOL.data()));
}

TEST(KernelSymbolsTest, ReadProcKallsyms)
{
  auto ks = read_proc_kallsyms();

  EXPECT_TRUE(ks.contains("security_sk_free"));
  EXPECT_TRUE(ks.contains("inet_release"));
  EXPECT_TRUE(ks.contains("tcp_connect"));
  EXPECT_TRUE(ks.contains("inet_csk_listen_start"));
  EXPECT_TRUE(ks.contains("tcp_init_sock"));
  EXPECT_TRUE(ks.contains("inet_csk_accept"));
  EXPECT_TRUE(ks.contains("udp_v4_get_port"));
  EXPECT_TRUE(ks.contains("udp_v6_get_port"));
  EXPECT_TRUE(ks.contains("tcp4_seq_show"));
  EXPECT_TRUE(ks.contains("tcp6_seq_show"));
  EXPECT_TRUE(ks.contains("udp4_seq_show"));
  EXPECT_TRUE(ks.contains("udp6_seq_show"));

  EXPECT_TRUE(ks.contains("taskstats_exit"));
  EXPECT_TRUE(ks.contains("cgroup_exit"));
  EXPECT_TRUE(ks.contains("cgroup_attach_task"));
  EXPECT_TRUE(ks.contains("wake_up_new_task"));
  EXPECT_TRUE(ks.contains("__set_task_comm"));
  EXPECT_TRUE(ks.contains("get_pid_task"));

  EXPECT_FALSE(ks.contains(UNKNOWN_SYMBOL.data()));
}

TEST(KernelSymbolsTest, NoSuchFile)
{
  EXPECT_THROW(read_proc_kallsyms("/NO_SUCH_FILE"), std::system_error);
}

TEST(KernelSymbolsTest, ParseError)
{
  std::stringstream stream("foo bar");
  EXPECT_THROW(read_proc_kallsyms(stream), std::runtime_error);
}

TEST(KernelSymbolsTest, NoParseError)
{
  std::stringstream stream("foo bar baz");
  EXPECT_NO_THROW(read_proc_kallsyms(stream));
}

TEST(KernelSymbolsTest, Empty)
{
  std::stringstream stream("\n\n\n");
  KernelSymbols ks;

  EXPECT_NO_THROW(ks = read_proc_kallsyms(stream));

  EXPECT_TRUE(ks.empty());
}
