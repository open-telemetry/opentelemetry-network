/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * C++-compatible container-of implementation.
 *
 * Inspired by https://stackoverflow.com/a/40851139
 */

#pragma once

#include <cstddef>

template <class P, class M> std::size_t fp_offsetof(const M P::*member)
{
  return (std::size_t) & (reinterpret_cast<P *>(0)->*member);
}

template <class P, class M> P *fp_container_of(M *ptr, const M P::*member)
{
  return (P *)((char *)ptr - fp_offsetof(member));
}
