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
//
// You can generate an h file from a live system using:
//    bpftool btf dump file /sys/kernel/btf/vmlinux format c
// (command extracted from https://github.com/aquasecurity/btfhub/blob/main/docs/btfgen-internals.md#btfhub)

#pragma once

// this is used to apply the required attribute to all structs
#pragma clang attribute push(__attribute__((preserve_access_index)), apply_to = record)

#define rcu_head callback_head

union nf_inet_addr {
  __u32 all[4];
  __be32 ip;
  __be32 ip6[4];
  struct in_addr in;
  struct in6_addr in6;
};

union nf_conntrack_man_proto {
  __be16 all;
};

struct nf_conntrack_man {
  union nf_inet_addr u3;
  union nf_conntrack_man_proto u;
  uint16_t l3num;
};

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
};

struct nf_conntrack_tuple_hash {
  struct hlist_nulls_node hnnode;
  struct nf_conntrack_tuple tuple;
};

struct nf_conn {
  struct nf_conntrack_tuple_hash tuplehash[2];
};

// Optional sk_buff field flavor for kernels with NF_CONNTRACK configured.
struct sk_buff___with_nfct {
  unsigned long _nfct;
};

struct cgroup;
struct cgroup_subsys;

struct cftype {
  char name[64];
  long unsigned int private;
  size_t max_write_len;
  unsigned int flags;
  unsigned int file_offset;
  struct cgroup_subsys *ss;
  struct list_head node;
  struct kernfs_ops *kf_ops;
};

struct cgroup_subsys {
  struct cgroup_subsys_state *(*css_alloc)(struct cgroup_subsys_state *);
  bool early_init : 1;
  bool implicit_on_dfl : 1;
  bool threaded : 1;
  int id;
  const char *name;
  const char *legacy_name;
  struct cgroup_root *root;
  struct idr css_idr;
  struct list_head cfts;
  struct cftype *dfl_cftypes;
  struct cftype *legacy_cftypes;
  unsigned int depends_on;
};

struct cgroup_subsys_state {
  struct cgroup *cgroup;
  struct cgroup_subsys *ss;
  struct percpu_ref refcnt;
  struct list_head sibling;
  struct list_head children;
  struct list_head rstat_css_node;
  int id;
  unsigned int flags;
  u64 serial_nr;
  atomic_t online_cnt;
  struct work_struct destroy_work;
  struct rcu_work destroy_rwork;
  struct cgroup_subsys_state *parent;
};

struct cgroup {
  struct cgroup_subsys_state self;
  long unsigned int flags;
  int level;
  int max_depth;
  int nr_descendants;
  int nr_dying_descendants;
  int max_descendants;
  int nr_populated_csets;
  int nr_populated_domain_children;
  int nr_populated_threaded_children;
  int nr_threaded_children;
  struct kernfs_node *kn;
  // struct cgroup_file procs_file;
  // struct cgroup_file events_file;
  // struct cgroup_file psi_files[3];
  u16 subtree_control;
  u16 subtree_ss_mask;
  u16 old_subtree_control;
  u16 old_subtree_ss_mask;
  struct cgroup_subsys_state *subsys[14];
  struct cgroup_root *root;
  struct list_head cset_links;
  struct list_head e_csets[14];
  struct cgroup *dom_cgrp;
  struct cgroup *old_dom_cgrp;
  // struct cgroup_rstat_cpu *rstat_cpu;
  struct list_head rstat_css_list;
  long : 64;
  long : 64;
  // struct cacheline_padding _pad_;
  struct cgroup *rstat_flush_next;
  // struct cgroup_base_stat last_bstat;
  // struct cgroup_base_stat bstat;
  struct prev_cputime prev_cputime;
  struct list_head pidlists;
  struct mutex pidlist_mutex;
  wait_queue_head_t offline_waitq;
  struct work_struct release_agent_work;
  struct psi_group *psi;
  // struct cgroup_bpf bpf;
  atomic_t congestion_count;
  // struct cgroup_freezer_state freezer;
  struct bpf_local_storage *bpf_cgrp_storage;
  struct cgroup *ancestors[0];
};

struct cgroup_root {
  struct kernfs_root *kf_root;
  unsigned int subsys_mask;
  int hierarchy_id;
  struct list_head root_list;
  struct callback_head rcu;
  long : 64;
  long : 64;
  struct cgroup cgrp;
  struct cgroup *cgrp_ancestor_storage;
  atomic_t nr_cgrps;
  unsigned int flags;
  char release_agent_path[4096];
  char name[64];
  long : 64;
  long : 64;
  long : 64;
  long : 64;
  long : 64;
  long : 64;
};

struct css_set {
  struct cgroup_subsys_state *subsys[14];
  refcount_t refcount;
  struct css_set *dom_cset;
  struct cgroup *dfl_cgrp;
  int nr_tasks;
  struct list_head tasks;
  struct list_head mg_tasks;
  struct list_head dying_tasks;
  struct list_head task_iters;
  struct list_head e_cset_node[14];
  struct list_head threaded_csets;
  struct list_head threaded_csets_node;
  struct hlist_node hlist;
  struct list_head cgrp_links;
  struct list_head mg_src_preload_node;
  struct list_head mg_dst_preload_node;
  struct list_head mg_node;
  struct cgroup *mg_src_cgrp;
  struct cgroup *mg_dst_cgrp;
  struct css_set *mg_dst_cset;
  bool dead;
  struct callback_head callback_head;
};

struct task_struct___with_css_set {
  struct css_set *cgroups;
};

// dynamically generated by the kernel based on config variables, read with bpf_core_enum_value
enum cgroup_subsys_id { mem_cgroup_subsys_id, memory_cgrp_id };

// remove the instruction to add preserve_access_index
#pragma clang attribute pop
