// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

namespace data {

template <typename Out, typename T> Out &&operator<<(Out &&out, Counter<T> const &what)
{
  if (what.empty()) {
    out << "{no_value}";
  } else {
    out << "{value=" << what.value() << '}';
  }
  return std::forward<Out>(out);
}

} // namespace data
