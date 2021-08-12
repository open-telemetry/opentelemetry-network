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

#include <collector/kernel/nat_prober.h>
#include <collector/kernel/probe_handler.h>
#include <ctime>
#include <iostream>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <util/log.h>

#define RCV_BUFFSIZE 8192 // see libnfnetlink/include/libnfnetlink.h for NFNL_BUFFSIZE

NatProber::NatProber(ProbeHandler &probe_handler, ebpf::BPFModule &bpf_module, std::function<void(void)> periodic_cb)
    : periodic_cb_(periodic_cb)
{
  // END
  probe_handler.start_probe(bpf_module, "on_nf_nat_cleanup_conntrack", "nf_nat_cleanup_conntrack");
  periodic_cb();

  // START
  probe_handler.start_probe(bpf_module, "on_nf_conntrack_alter_reply", "nf_conntrack_alter_reply");
  periodic_cb();

  // EXISTING
  probe_handler.start_probe(bpf_module, "on_ctnetlink_dump_tuples", "ctnetlink_dump_tuples");
  periodic_cb();
  int res = query_kernel();
  if (res != 0) {
    if (res == EAGAIN || res == EWOULDBLOCK) {
      LOG::warn(
          "While probing NAT, netfilter socket finished before seeing "
          "NLMSG_DONE. {}",
          std::strerror(res));
    } else {
      LOG::error("NatProber::NatProber() - Error calling query_kernel(): {}", std::strerror(res));
    }
  }
  periodic_cb();

  // Cleanup existing
  probe_handler.cleanup_probe("p_ctnetlink_dump_tuples");
  periodic_cb();
}

/* Creates a netlink socket and creates a request for the kernel to dump its
 * conntrack table information. Returns 0 on success and errno on failure.
 */
int NatProber::query_kernel()
{
  // create a netlink socket
  // domain: AF_NETLINK, type: SOCK_RAW | SOCK_NONBLOCK, protocol:
  // NETLINK_NETFILTER
  int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK, NETLINK_NETFILTER);
  if (sock_fd == -1) {
    LOG::debug("NatProber::query_kernel() - Error opening netlink socket: {}", std::strerror(errno));
    return errno;
  }

  // Source addr info (where to bind)
  struct sockaddr_nl src_addr;
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid(); // self pid
  src_addr.nl_groups = 0;     // unicast
  int err = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
  if (err == -1) {
    int saved_errno = errno;
    LOG::debug("NatProber::query_kernel() - Error binding netlink socket: {}", std::strerror(saved_errno));
    close(sock_fd);
    return saved_errno;
  }

  // Dest addr_info (where to send)
  struct sockaddr_nl dst_addr;
  memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.nl_family = AF_NETLINK;
  dst_addr.nl_pid = 0;    // for the linux kernel
  dst_addr.nl_groups = 0; // unicast

  // Setup a netlink/conntrack query message - this is based on the msg created
  // in the conntrack tool. Specifically the codepath triggered by `sudo
  // conntrack -L`
  struct nlct_query_msg {
    struct nlmsghdr nlh;
    struct nfgenmsg nfmsg;
    struct nfattr nfattr1;
    u32 nfattr_data1;
    struct nfattr nfattr2;
    u32 nfattr_data2;
  };
  struct nlct_query_msg msg;
  memset(&msg, 0, sizeof(msg));

  // nlh
  msg.nlh.nlmsg_len = sizeof(msg); // size of the entire msg
  msg.nlh.nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_GET;
  msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  msg.nlh.nlmsg_seq = (u32)std::time(nullptr); // current unix time
  msg.nlh.nlmsg_pid = 0;                       // for the linux kernel

  // nfmsg
  msg.nfmsg.nfgen_family = AF_INET;
  msg.nfmsg.version = NFNETLINK_V0;
  msg.nfmsg.res_id = 0;

  // nfattrs
  msg.nfattr1.nfa_len = 0x08; // size of nfattr1 + nfattr_data1
  msg.nfattr1.nfa_type = CTA_MARK;
  msg.nfattr_data1 = 0;       // empty
  msg.nfattr2.nfa_len = 0x08; // size of nfattr2 + nfattr_data2
  msg.nfattr2.nfa_type = CTA_MARK_MASK;
  msg.nfattr_data2 = 0; // empty

  // Send msg
  err = sendto(sock_fd, (void *)&msg, sizeof(msg), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
  if (err == -1) {
    int saved_errno = errno;
    LOG::debug("NatProber::query_kernel() - Error sending on netlink socket: {}", std::strerror(saved_errno));
    close(sock_fd);
    return saved_errno;
  }

  // Receive responses
  while (1) {
    socklen_t addrlen = sizeof(dst_addr);
    unsigned char buf[RCV_BUFFSIZE];
    err = recvfrom(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&dst_addr, &addrlen);
    if (err == -1) {
      int saved_errno = errno;
      LOG::debug("NatProber::query_kernel() - Error receiving on netlink socket: {}", std::strerror(saved_errno));
      close(sock_fd);
      return saved_errno;
    }
    // If no data was returned then we break - but this is an unexpected case,
    // so log for debugging purposes.
    if (err == 0) {
      LOG::debug("NatProber::query_kernel() - recvfrom = 0");
      break;
      ;
    }
    // Break once we're done receiving messages
    // According to netlink_dump() inside of netlink/af_netlink.c, we expect the
    // last msg in a dump to consist of only an nlmsghdr with a flag for
    // NLMSG_DONE.
    struct nlmsghdr *nlh = (struct nlmsghdr *)buf;
    if (nlh->nlmsg_type == NLMSG_DONE) {
      break;
    }

    // Ensure we handle perf events in a timely fashion
    // else we run the risk of overflowing the event buffer
    periodic_cb_();
  }

  // Cleanup
  close(sock_fd);
  return 0;
}
