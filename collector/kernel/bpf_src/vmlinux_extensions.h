/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

 // This file contains kernel data structures that are not in libbpf's regular vmlinux.h repository
// (https://github.com/libbpf/vmlinux.h).
//
// libbpf's vmlinux.h is prepared with their own config, not from distros:
//  - https://github.com/libbpf/vmlinux.h/blob/main/.github/workflows/vmlinux.h.yml
//  - https://github.com/libbpf/vmlinux.h/blob/main/scripts/gen-vmlinux-header.sh
//  - https://github.com/libbpf/vmlinux.h/blob/main/kconfigs/config.x86_64
// and so contains partial structures.
//
// This file adds more that is required by the project. These structs must have __attribute__((preserve_access_index));
// See "Defining own CO-RE-relocatable type definitions" in https://nakryiko.com/posts/bpf-core-reference-guide/

#pragma once

struct nf_conn {
  struct nf_conntrack_tuple_hash tuplehash[2];
} __attribute__((preserve_access_index));

union nf_inet_addr {
	__u32		all[4];
	__be32		ip;
	__be32		ip6[4];
	struct in_addr	in;
	struct in6_addr	in6;
} __attribute__((preserve_access_index));

union nf_conntrack_man_proto {
  __be16 all;
 } __attribute__((preserve_access_index));

struct nf_conntrack_man {
	union nf_inet_addr u3;
	union nf_conntrack_man_proto u;
} __attribute__((preserve_access_index));

struct nf_conntrack_tuple {
  struct nf_conntrack_man src;
  struct {
    union nf_inet_addr u3;
    union {
			__be16 all;
		} u;

    u_int8_t protonum;
		u_int8_t dir;
  } dst;
} __attribute__((preserve_access_index));
