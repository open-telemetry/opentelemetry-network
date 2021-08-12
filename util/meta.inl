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
