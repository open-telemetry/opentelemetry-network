/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

// This file contains old struct definitions for compatibility

#pragma once

// required for btf relocations
#pragma clang attribute push (__attribute__((preserve_access_index)), apply_to = record)

struct msghdr___3_18_140 {
	void		*msg_name;	/* ptr to socket address structure */
	int		msg_namelen;	/* size of socket address structure */
	struct iovec	*msg_iov;	/* scatter/gather array */
	__kernel_size_t	msg_iovlen;	/* # elements in msg_iov */
	void		*msg_control;	/* ancillary data */
	__kernel_size_t	msg_controllen;	/* ancillary data buffer length */
	unsigned int	msg_flags;	/* flags on received message */
};

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
	void		*msg_name;	/* ptr to socket address structure */
	int		msg_namelen;	/* size of socket address structure */
	struct iov_iter___5_13_19	msg_iter;	/* data */

	/*
	 * Ancillary data. msg_control_user is the user buffer used for the
	 * recv* side when msg_control_is_user is set, msg_control is the kernel
	 * buffer used for all other cases.
	 */
	union {
		void		*msg_control;
		void __user	*msg_control_user;
	};
	bool		msg_control_is_user : 1;
	__kernel_size_t	msg_controllen;	/* ancillary data buffer length */
	unsigned int	msg_flags;	/* flags on received message */
	struct kiocb	*msg_iocb;	/* ptr to iocb for async requests */
};

// remove the instruction to add preserve_access_index
#pragma clang attribute pop
