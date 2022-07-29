// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

namespace meta {

// implementation for meta::foreach

template <template <typename...> typename List, typename... T, typename Fn, typename... Args>
std::size_t foreach_impl(meta::tag<List<T...>>, Fn &fn, Args &... args)
{
  // uses the fold expression for operator+ to expand the elements of the type
  // list and guarantee that `fn` will be called in the correct order for each
  // element of the type list
  return ((fn(meta::tag<T>{}, args...), true) && ... && true);
}

template <typename List, typename Fn, typename... Args> void foreach (Fn &&fn, Args && ... args)
{
  meta::foreach_impl(tag<List>{}, fn, args...);
}

} // namespace meta
