/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

// This file contains old struct definitions for compatibility

#pragma once

// required for btf relocations
#pragma clang attribute push(__attribute__((preserve_access_index)), apply_to = record)

struct iov_iter___5_13_19 {
  /*
   * Bit 0 is the read/write bit, set if we're writing.
   * Bit 1 is the BVEC_FLAG_NO_REF bit, set if type is a bvec and
   * the caller isn't expecting to drop a page reference when done.
   */
  unsigned int type;
  size_t iov_offset;
  size_t count;
  union {
    const struct iovec *iov;
    const struct kvec *kvec;
    const struct bio_vec *bvec;
    struct xarray *xarray;
    struct pipe_inode_info *pipe;
  };
  union {
    unsigned long nr_segs;
    struct {
      unsigned int head;
      unsigned int start_head;
    };
    loff_t xarray_start;
  };
};

struct msghdr___5_13_19 {
  void *msg_name;                     /* ptr to socket address structure */
  int msg_namelen;                    /* size of socket address structure */
  struct iov_iter___5_13_19 msg_iter; /* data */

  /*
   * Ancillary data. msg_control_user is the user buffer used for the
   * recv* side when msg_control_is_user is set, msg_control is the kernel
   * buffer used for all other cases.
   */
  union {
    void *msg_control;
    // void __user	*msg_control_user;
  };
  bool msg_control_is_user : 1;
  __kernel_size_t msg_controllen; /* ancillary data buffer length */
  unsigned int msg_flags;         /* flags on received message */
  struct kiocb *msg_iocb;         /* ptr to iocb for async requests */
};

struct css_id___3_11 {
  /*
   * The css to which this ID points. This pointer is set to valid value
   * after cgroup is populated. If cgroup is removed, this will be NULL.
   * This pointer is expected to be RCU-safe because destroy()
   * is called after synchronize_rcu(). But for safe use, css_tryget()
   * should be used for avoiding race.
   */
  struct cgroup_subsys_state *css;
  /*
   * ID of this css.
   */
  unsigned short id;
  /*
   * Depth in hierarchy which this ID belongs to.
   */
  unsigned short depth;
  /*
   * ID is freed by RCU. (and lookup routine is RCU safe.)
   */
  struct rcu_head rcu_head;
  /*
   * Hierarchy of CSS ID belongs to.
   */
  unsigned short stack[0]; /* Array of Length (depth+1) */
};

struct cgroup_name___3_11 {
  struct rcu_head rcu_head;
  char name[];
};

struct cgroup___3_11 {
  unsigned long flags; /* "unsigned long" so bitops work */

  int id; /* ida allocated in-hierarchy ID */

  /*
   * We link our 'sibling' struct into our parent's 'children'.
   * Our children link their 'sibling' into our 'children'.
   */
  struct list_head sibling;  /* my parent's children */
  struct list_head children; /* my children */
  struct list_head files;    /* my files */

  struct cgroup *parent; /* my parent */
  struct dentry *dentry; /* cgroup fs entry, RCU protected */

  /*
   * Monotonically increasing unique serial number which defines a
   * uniform order among all cgroups.  It's guaranteed that all
   * ->children lists are in the ascending order of ->serial_nr.
   * It's used to allow interrupting and resuming iterations.
   */
  u64 serial_nr;

  /*
   * This is a copy of dentry->d_name, and it's needed because
   * we can't use dentry->d_name in cgroup_path().
   *
   * You must acquire rcu_read_lock() to access cgrp->name, and
   * the only place that can change it is rename(), which is
   * protected by parent dir's i_mutex.
   *
   * Normally you should use cgroup_name() wrapper rather than
   * access it directly.
   */
  struct cgroup_name___3_11 *name;

  /* Private pointers for each registered subsystem */
  // struct cgroup_subsys_state *subsys[CGROUP_SUBSYS_COUNT];

  struct cgroupfs_root *root;

  /*
   * List of cgrp_cset_links pointing at css_sets with tasks in this
   * cgroup.  Protected by css_set_lock.
   */
  struct list_head cset_links;

  /*
   * Linked list running through all cgroups that can
   * potentially be reaped by the release agent. Protected by
   * release_list_lock
   */
  struct list_head release_list;

  /*
   * list of pidlists, up to two for each namespace (one for procs, one
   * for tasks); created on demand.
   */
  struct list_head pidlists;
  struct mutex pidlist_mutex;

  /* For css percpu_ref killing and RCU-protected deletion */
  struct rcu_head rcu_head;
  struct work_struct destroy_work;
  atomic_t css_kill_cnt;

  /* List of events which userspace want to receive */
  struct list_head event_list;
  spinlock_t event_list_lock;

  /* directory xattrs */
  struct simple_xattrs xattrs;
};

struct cgroup_subsys_state___3_11 {
  /*
   * The cgroup that this subsystem is attached to. Useful
   * for subsystems that want to know about the cgroup
   * hierarchy structure
   */
  struct cgroup___3_11 *cgroup;

  /* reference count - access via css_[try]get() and css_put() */
  struct percpu_ref refcnt;

  unsigned long flags;
  /* ID for this css, if possible */
  struct css_id___3_11 *id;

  /* Used to put @cgroup->dentry on the last css_put() */
  struct work_struct dput_work;
};

struct tcp_sock___rcv_rtt_est_rtt {
  struct {
    u32 rtt;
    u32 seq;
    u32 time;
  } rcv_rtt_est;
};

// remove the instruction to add preserve_access_index
#pragma clang attribute pop
