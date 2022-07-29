/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * linux-compat.h
 */

#ifndef FASTPASS_PLATFORM_GENERIC_H_
#define FASTPASS_PLATFORM_GENERIC_H_

#include <platform/types.h>

#define fp_fprintf_nonz_prefix(f, var, description, prefix)                                                                    \
  {                                                                                                                            \
    if (var)                                                                                                                   \
      fp_fprintf(f, prefix "%12llu  " description "\n", (unsigned long long)(var));                                            \
  }

#define fp_fprintf_nonz(f, var, description) fp_fprintf_nonz_prefix(f, var, description, "  ")

#ifdef __KERNEL__

/**
 * Works around the requirement for KBUILD_MODNAME to be defined when using
 *   pr_debug(). Some kernel header files use pr_debug(), and they fail in our
 *   "infrastructure" code, i.e., the files that are included from multiple
 *   modules (see scripts/Makefile.lib).
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "fastpass_common"
#endif /* #ifndef KBUILD_MODNAME */
#endif /* defined(CONFIG_DYNAMIC_DEBUG) */

#include <linux/errno.h>
#include <linux/jhash.h>
#include <linux/jiffies.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <net/ip.h>

#ifndef time_in_range64
#define time_in_range64(a, b, c) (time_after_eq64(a, b) && time_before_eq64(a, c))
#endif

typedef struct seq_file fp_outfile;
#define fp_fprintf(f, ...) seq_printf(f, __VA_ARGS__)

#define fp_vzalloc(size) vzalloc(size)
#define fp_vfree(ptr) vfree(ptr)

#else

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE fp_outfile;
#define fp_fprintf(f, ...) fprintf(f, __VA_ARGS__)

#define fp_vzalloc(size) calloc(1, size)
#define fp_vfree(ptr) free(ptr)

/* compiler-gcc.h */
#ifdef __GNUC__
#define barrier() __asm__ __volatile__("" : : : "memory")
#define mb() asm volatile("mfence" : : : "memory")
#define rmb() asm volatile("lfence" : : : "memory")
#define wmb() asm volatile("sfence" : : : "memory")
#define read_barrier_depends()                                                                                                 \
  do {                                                                                                                         \
  } while (0)

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif

#define smp_wmb() mb()
#define smp_mb() barrier()
#define smp_rmb() barrier()
#define smp_read_barrier_depends() read_barrier_depends()

#ifdef RTE_ARCH
#include <rte_byteorder.h>
#else
#include <arpa/inet.h>
#endif

#ifdef RTE_ARCH

#include <rte_branch_prediction.h>

#else
#ifndef unlikely
#define unlikely(x) (x)
#endif

#ifndef likely
#define likely(x) (x)
#endif
#endif

#ifdef RTE_ARCH
#include <rte_common.h>
#else
#ifndef typeof
#define typeof __typeof__
#endif
#endif

#define CONFIG_64BIT

/* from kernel.h */
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

/* this is a way to compute this without platform-dependent code */
#define BITS_PER_LONG (BITS_PER_BYTE * sizeof(long))

/* from bitops.h */
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

/* include/asm-generic/bitops/non-atomic.h */
static inline void __set_bit(int nr, unsigned long *addr)
{
  unsigned long mask = BIT_MASK(nr);
  unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

  *p |= mask;
}

static inline void __clear_bit(int nr, unsigned long *addr)
{
  unsigned long mask = BIT_MASK(nr);
  unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

  *p &= ~mask;
}

static inline int test_bit(int nr, const volatile unsigned long *addr)
{
  return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1)));
}

/* kernel.h */
#define max_t(type, x, y)                                                                                                      \
  ({                                                                                                                           \
    type __max1 = (x);                                                                                                         \
    type __max2 = (y);                                                                                                         \
    __max1 > __max2 ? __max1 : __max2;                                                                                         \
  })

/* typecheck.h */
#define typecheck(type, x)                                                                                                     \
  ({                                                                                                                           \
    type __dummy;                                                                                                              \
    typeof(x) __dummy2;                                                                                                        \
    (void)(&__dummy == &__dummy2);                                                                                             \
    1;                                                                                                                         \
  })

/* jiffies.h */
#define time_after64(a, b) (typecheck(u64, a) && typecheck(u64, b) && ((s64)((b) - (a)) < 0))
#define time_before64(a, b) time_after64(b, a)

#define time_after_eq64(a, b) (typecheck(u64, a) && typecheck(u64, b) && ((s64)((a) - (b)) >= 0))
#define time_before_eq64(a, b) time_after_eq64(b, a)

#define time_in_range64(a, b, c) (time_after_eq64(a, b) && time_before_eq64(a, c))

/* need __fls */
#define __fls(x) (BITS_PER_LONG - 1 - __builtin_clzl(x))
#define __ffs(x) (__builtin_ffsl(x) - 1)

#define csum_partial fp_csum_partial
#define csum_tcpudp_magic fp_csum_tcpudp_magic

/* based on rte_hash_crc from DPDK's rte_hash_crc.h, but does checksum */
static inline uint32_t fp_csum_partial(const void *data, uint32_t data_len, uint32_t init_val)
{
  unsigned i;
  u64 sum = init_val;
  const uint32_t *p32 = (const uint32_t *)data;
  bool flip = false;

  if (unlikely(data_len < 4))
    goto do_last;

  flip = (u64)p32 & 0x1;
  if (unlikely(flip)) {
    sum += *((const uint8_t *)p32) << 8;
    p32 = (const uint32_t *)((const uint8_t *)p32 + 1);
    data_len -= 1;
  }

  if ((u64)p32 & 0x2) {
    sum += *((const uint16_t *)p32);
    p32 = (const uint32_t *)((const uint8_t *)p32 + 2);
    data_len -= 2;
  }

  for (i = 0; i < data_len / 4; i++) {
    sum += *p32++;
  }

do_last:
  switch (3 - (data_len & 0x03)) {
  case 0:
    sum += *((const uint8_t *)p32 + 2) << 16;
    /* Fallthrough */
  case 1:
    sum += *((const uint8_t *)p32 + 1) << 8;
    /* Fallthrough */
  case 2:
    sum += *((const uint8_t *)p32);
  default:
    break;
  }

  if (unlikely(flip))
    sum <<= 8; /* assume we didn't checksum so much data to overflow 56 bits */

  sum = (u32)sum + (sum >> 32);       /* could have overflow on bit 32 */
  return (u32)sum + (u32)(sum >> 32); /* add the overflow */
}

static inline uint16_t fp_fold(uint64_t sum64)
{
  uint32_t sum32;

  /* now fold */
  sum64 = (u32)sum64 + (sum64 >> 32);        /* could have overflow on bit 32 */
  sum32 = sum64 + (sum64 >> 32);             /* add the overflow */
  sum32 = (u16)sum32 + (sum32 >> 16);        /* could have overflow on bit 16 */
  return ~((u16)sum32 + (u16)(sum32 >> 16)); /* add the overflow */
}

static inline uint16_t fp_csum_tcpudp_magic(uint32_t saddr, uint32_t daddr, uint16_t len, uint16_t proto, uint32_t sum32)
{
  uint64_t sum64 = sum32;
  sum64 += saddr + daddr + ((len + proto) << 8);

  /* now fold */
  return fp_fold(sum64);
}

#endif /* __KERNEL__ */

static inline void __set_bit64(int nr, u64 *addr)
{
  u64 *p = (addr + (nr / 64));
  *p |= (1ULL << (nr % 64));
}

static inline void __clear_bit64(int nr, u64 *addr)
{
  u64 *p = (addr + (nr / 64));
  *p &= ~(1ULL << (nr % 64));
}

/* from Jenkin's public domain lookup3.c at
 * http://burtleburtle.net/bob/c/lookup3.c */
#define jhash_3words fp_jhash_3words
#define jhash_1word fp_jhash_1word

#define fp_rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))
#define fp_jhash_final(a, b, c)                                                                                                \
  {                                                                                                                            \
    c ^= b;                                                                                                                    \
    c -= fp_rot(b, 14);                                                                                                        \
    a ^= c;                                                                                                                    \
    a -= fp_rot(c, 11);                                                                                                        \
    b ^= a;                                                                                                                    \
    b -= fp_rot(a, 25);                                                                                                        \
    c ^= b;                                                                                                                    \
    c -= fp_rot(b, 16);                                                                                                        \
    a ^= c;                                                                                                                    \
    a -= fp_rot(c, 4);                                                                                                         \
    b ^= a;                                                                                                                    \
    b -= fp_rot(a, 14);                                                                                                        \
    c ^= b;                                                                                                                    \
    c -= fp_rot(b, 24);                                                                                                        \
  }

/* compatibility with the kernel's jhash -- note this is only compatible
 * with _some_ linux versions */
static inline u32 fp_jhash_nwords(u32 a, u32 b, u32 c, u32 initval)
{
  a += initval;
  b += initval;
  c += initval;
  fp_jhash_final(a, b, c);
  return c;
}
static inline u32 fp_jhash_3words(u32 a, u32 b, u32 c, u32 initval)
{
  return fp_jhash_nwords(a, b, c, initval + 0xDEADBEEF + (3 << 2));
}
static inline u32 fp_jhash_1word(u32 a, u32 initval)
{
  return fp_jhash_nwords(a, 0, 0, initval + 0xDEADBEEF + (1 << 2));
}

#endif /* FASTPASS_PLATFORM_GENERIC_H_ */
