// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <climits>

namespace views {

inline std::size_t run_length(std::string_view input, std::string_view what)
{
  auto const index = input.find_first_not_of(what);
  return index == std::string_view::npos ? input.size() : index;
}

inline std::string_view trim_run(std::string_view &input, std::string_view what)
{
  auto const count = run_length(input, what);
  assert(count <= input.size());
  auto result = input.substr(0, count);
  input.remove_prefix(count);
  return result;
}

inline std::size_t count_up_to(std::string_view input, char what, bool include)
{
  auto const index = input.find(what);
  return index == std::string_view::npos ? input.size() : index + include;
}

inline std::size_t count_up_to(std::string_view input, std::string_view what, bool include)
{
  auto const index = input.find_first_of(what);
  return index == std::string_view::npos ? input.size() : index + include;
}

inline std::string_view trim_up_to(std::string_view &input, char what, SeekBehavior behavior)
{
  auto const index = input.find(what);
  if (index == std::string_view::npos) {
    auto result = input;
    input = {};
    return result;
  } else {
    assert(index < input.size());

    auto const result_count = index + (static_cast<std::uint8_t>(behavior) & 0b01);
    assert(result_count <= input.size());
    auto result = input.substr(0, result_count);

    auto const remove_count = index + ((static_cast<std::uint8_t>(behavior) & 0b10) >> 1);
    assert(remove_count <= input.size());
    input.remove_prefix(remove_count);

    return result;
  }
}

inline std::string_view trim_up_to(std::string_view &input, std::string_view what, SeekBehavior behavior)
{
  auto const index = input.find_first_of(what);
  if (index == std::string_view::npos) {
    auto result = input;
    input = {};
    return result;
  } else {
    assert(index < input.size());

    auto const result_count = index + (static_cast<std::uint8_t>(behavior) & 0b01);
    assert(result_count <= input.size());
    auto result = input.substr(0, result_count);

    auto const remove_count = index + ((static_cast<std::uint8_t>(behavior) & 0b10) >> 1);
    assert(remove_count <= input.size());
    input.remove_prefix(remove_count);

    return result;
  }
}

inline std::size_t count_up_to_last(std::string_view input, char what, bool include)
{
  auto const index = input.rfind(what);
  assert(index == std::string_view::npos || index < input.size());
  return index == std::string_view::npos ? 0 : index + include;
}

inline std::size_t count_up_to_last(std::string_view input, std::string_view what, bool include)
{
  auto const index = input.find_last_of(what);
  assert(index == std::string_view::npos || index < input.size());
  return index == std::string_view::npos ? 0 : index + include;
}

inline std::string_view trim_up_to_last(std::string_view &input, char what, SeekBehavior behavior)
{
  auto const index = input.rfind(what);
  if (index == std::string_view::npos) {
    return std::string_view{};
  } else {
    assert(index < input.size());

    auto const result_count = index + (static_cast<std::uint8_t>(behavior) & 0b01);
    assert(result_count <= input.size());
    auto result = input.substr(0, result_count);

    auto const remove_count = index + ((static_cast<std::uint8_t>(behavior) & 0b10) >> 1);
    assert(remove_count <= input.size());
    input.remove_prefix(remove_count);

    return result;
  }
}

inline std::string_view trim_up_to_last(std::string_view &input, std::string_view what, SeekBehavior behavior)
{
  auto const index = input.find_last_of(what);
  if (index == std::string_view::npos) {
    return std::string_view{};
  } else {
    assert(index < input.size());

    auto const result_count = index + (static_cast<std::uint8_t>(behavior) & 0b01);
    assert(result_count <= input.size());
    auto result = input.substr(0, result_count);

    auto const remove_count = index + ((static_cast<std::uint8_t>(behavior) & 0b10) >> 1);
    assert(remove_count <= input.size());
    input.remove_prefix(remove_count);

    return result;
  }
}

inline std::string_view slice_suffix(std::string_view view, std::size_t size)
{
  if (view.size() > size) {
    view.remove_prefix(view.size() - size);
  }

  return view;
}

inline bool ends_with(std::string_view view, std::string_view suffix)
{
  if (view.length() < suffix.length()) {
    return false;
  }

  return view.substr(view.length() - suffix.length()) == suffix;
}

template <typename T> constexpr T NumberView<T>::value(T fallback) const
{
  if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
    integer_from_string(view_.data(), fallback);
  } else {
    floating_point_from_string(view_.data(), fallback);
  }
  return fallback;
}

template <typename T> constexpr T NumberView<T>::unpadded_value(T fallback) const
{
  // correct size is `ceil(log_10(bit-width)) + 1`, but doing that math a
  // runtime defeats the purpose of a fast number view facility, and doing it
  // at compile time requires too much stuff to be implemented to be worth it
  constexpr std::size_t buffer_size = sizeof(T) * CHAR_BIT + 1;
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>, "TODO: compute correct buffer size for floating point parsing");

  char buffer[buffer_size] = {0};
  auto const end = std::min(buffer_size - 1, view_.size());
  view_.copy(buffer, end);

  if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
    integer_from_string(buffer, fallback);
  } else {
    floating_point_from_string(buffer, fallback);
  }

  return fallback;
}

} // namespace views
