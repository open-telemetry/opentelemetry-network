/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <platform/generic.h>

template <std::size_t SIZE> class IterableBitmap {
public:
  typedef std::size_t size_type;
  static constexpr size_type size = SIZE;
  /* at l0 we have one word for every 2^6 bits */
  static constexpr size_type l0 = ((size + 63) / 64);
  /* at l1 we have one word for every 2^12 bits */
  static constexpr size_type l1 = ((l0 == 1) ? 0 : ((l0 + 63) / 64));
  /* at l2 we have one word for every 2^18 bits */
  static constexpr size_type l2 = ((l1 == 1) ? 0 : ((l1 + 63) / 64));
  /* at l3 we have one word for every 2^24 bits */
  static constexpr size_type l3 = ((l2 == 1) ? 0 : ((l2 + 63) / 64));
  /* at l4 we have one word for every 2^30 bits */
  static constexpr size_type l4 = ((l3 == 1) ? 0 : ((l3 + 63) / 64));
  /* number of levels */
  static constexpr size_type levels = ((l4 > 0) ? 5 : ((l3 > 0) ? 4 : ((l2 > 0) ? 3 : ((l1 > 0) ? 2 : 1))));

  IterableBitmap() : mask0{}, mask1{}, mask2{}, mask3{}, mask4{}
  {
    static_assert(size > 0, "IterableBitmap must have > 0 elements");
    static_assert(l4 <= 1, "IterableBitmap supports <= (1<<30) elements");
  }

  class iterator {
  public:
    size_type operator*() const { return i[0]; }

    /* we only support != where rhs is end() */
    bool operator!=(const iterator &rhs) const { return mask[levels - 1] != 0; }

    iterator &operator++()
    {
      mask[0] &= mask[0] - 1;
      if (mask[0] == 0) {
        if (levels == 1)
          return *this;
        mask[1] &= mask[1] - 1;
        if (mask[1] == 0) {
          if (levels == 2)
            return *this;
          mask[2] &= mask[2] - 1;
          if (mask[2] == 0) {
            if (levels == 3)
              return *this;

            mask[3] &= mask[3] - 1;
            if (mask[3] == 0) {
              if (levels == 4)
                return *this;

              mask[4] &= mask[4] - 1;
              if (mask[4] == 0)
                /* levels must be == 5 */
                return *this;

              i[4] = __builtin_ctzll(mask[4]);
              mask[3] = bitmap->mask3[i[4]];
              i[4] <<= 6;
            }
            i[3] = __builtin_ctzll(mask[3]) + ((l4 > 0) ? i[4] : 0);
            mask[2] = bitmap->mask2[i[3]];
            i[3] <<= 6;
          }
          i[2] = __builtin_ctzll(mask[2]) + ((l3 > 0) ? i[3] : 0);
          mask[1] = bitmap->mask1[i[2]];
          i[2] <<= 6;
        }
        i[1] = __builtin_ctzll(mask[1]) + ((l2 > 0) ? i[2] : 0);
        mask[0] = bitmap->mask0[i[1]];
        i[1] <<= 6;
      }
      i[0] = __builtin_ctzll(mask[0]) + ((l1 > 0) ? i[1] : 0);
      return *this;
    }

  private:
    friend class IterableBitmap;
    const IterableBitmap *bitmap;
    std::array<size_type, levels> i;
    std::array<u64, levels> mask;
  };

  /**
   * Marks the given bit.
   * @assumes index < SIZE.
   */
  void set(size_type index)
  {
    __set_bit64(index, mask0.data());
    if (l1 > 0)
      __set_bit64(index >> 6, mask1.data());
    if (l2 > 0)
      __set_bit64(index >> 12, mask2.data());
    if (l3 > 0)
      __set_bit64(index >> 18, mask3.data());
    if (l4 > 0)
      __set_bit64(index >> 24, mask4.data());
  }

  /**
   * unmarks the given bit.
   * @assumes index < SIZE.
   */
  void clear(size_type index)
  {
    __clear_bit64(index, mask0.data());
    if ((l1 > 0) && (mask0[index >> 6] == 0))
      __clear_bit64(index >> 6, mask1.data());
    if ((l2 > 0) && (mask1[index >> 12] == 0))
      __clear_bit64(index >> 12, mask2.data());
    if ((l3 > 0) && (mask2[index >> 18] == 0))
      __clear_bit64(index >> 18, mask3.data());
    if ((l4 > 0) && (mask3[index >> 24] == 0))
      __clear_bit64(index >> 24, mask4.data());
  }

  int get(size_type index) const { return test_bit(index, mask0.data()); }

  iterator begin() const
  {
    iterator res;
    /* first, get the root */
    u64 root;
    if (l4 > 0) {
      root = mask4[0];
    } else if (l3 > 0) {
      root = mask3[0];
    } else if (l2 > 0) {
      root = mask2[0];
    } else if (l1 > 0) {
      root = mask1[0];
    } else /* l0 > 0 */ {
      root = mask0[0];
    }
    res.mask[levels - 1] = root;

    /* empty set? */
    if (root == 0)
      return res;

    res.bitmap = this;

    /* now resolve the masks and indices on the path to the leaf */
    if (l4 > 0) {
      res.i[4] = __builtin_ctzll(res.mask[4]);
      res.mask[3] = mask3[res.i[4]];
      res.i[4] <<= 6;
    }
    if (l3 > 0) {
      res.i[3] = __builtin_ctzll(res.mask[3]) + ((l4 > 0) ? res.i[4] : 0);
      res.mask[2] = mask2[res.i[3]];
      res.i[3] <<= 6;
    }
    if (l2 > 0) {
      res.i[2] = __builtin_ctzll(res.mask[2]) + ((l3 > 0) ? res.i[3] : 0);
      res.mask[1] = mask1[res.i[2]];
      res.i[2] <<= 6;
    }
    if (l1 > 0) {
      res.i[1] = __builtin_ctzll(res.mask[1]) + ((l2 > 0) ? res.i[2] : 0);
      ;
      res.mask[0] = mask0[res.i[1]];
      res.i[1] <<= 6;
    }
    res.i[0] = __builtin_ctzll(res.mask[0]) + ((l1 > 0) ? res.i[1] : 0);
    return res;
  }

  iterator end() const { return iterator(); }

private:
  friend class iterator;
  std::array<u64, l0> mask0;
  std::array<u64, l1> mask1;
  std::array<u64, l2> mask2;
  std::array<u64, l3> mask3;
  std::array<u64, l4> mask4;
};
