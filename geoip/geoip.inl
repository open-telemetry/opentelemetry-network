// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

// this file shouldn't be included by itself
// it hides implementation details from the API header

#pragma once

#include <util/short_string.h>

#include <algorithm>

namespace geoip {

/**
 * Helper for proper data type handling in GeoIP data units.
 */
template <typename> struct data_type;

template <> struct data_type<std::string_view> {
  using type = std::string_view;

  /**
   * Returns true if `data_type::type` can represent `data.type`.
   */
  static bool same(MMDB_entry_data_s const &data)
  {
    return data.type == MMDB_DATA_TYPE_BYTES || data.type == MMDB_DATA_TYPE_UTF8_STRING;
  }

  /**
   * Converts the internal data in `data` and returns it as an instance of `data_type::type`.
   */
  static std::string_view get(MMDB_entry_data_s const &data)
  {
    auto string = data.type == MMDB_DATA_TYPE_BYTES ? reinterpret_cast<char const *>(data.bytes) : data.utf8_string;

    return {string, data.data_size};
  }
};

template <> struct data_type<std::string> {
  using type = std::string;

  /**
   * Returns true if `data_type::type` can represent `data.type`.
   */
  static bool same(MMDB_entry_data_s const &data)
  {
    return data.type == MMDB_DATA_TYPE_BYTES || data.type == MMDB_DATA_TYPE_UTF8_STRING;
  }

  /**
   * Converts the internal data in `data` and returns it as an instance of `data_type::type`.
   */
  static std::string get(MMDB_entry_data_s const &data)
  {
    auto string = data.type == MMDB_DATA_TYPE_BYTES ? reinterpret_cast<char const *>(data.bytes) : data.utf8_string;

    return std::string(string, data.data_size);
  }
};

template <std::size_t N> struct data_type<short_string<N>> {
  using type = short_string<N>;

  /**
   * Returns true if `data_type::type` can represent `data.type`.
   */
  static bool same(MMDB_entry_data_s const &data)
  {
    return data.type == MMDB_DATA_TYPE_BYTES || data.type == MMDB_DATA_TYPE_UTF8_STRING;
  }

  /**
   * Converts the internal data in `data` and returns it as an instance of `data_type::type`.
   */
  static short_string<N> get(MMDB_entry_data_s const &data)
  {
    auto string = (data.type == MMDB_DATA_TYPE_BYTES) ? reinterpret_cast<char const *>(data.bytes) : data.utf8_string;

    return short_string<N>(string, std::min<std::size_t>(data.data_size, N));
  }
};

#define REGISTER_GEOIP_DATA_TYPE(T, Type, Member)                                                                              \
  template <> struct data_type<T> {                                                                                            \
    using type = T;                                                                                                            \
    static bool same(MMDB_entry_data_s const &data)                                                                            \
    {                                                                                                                          \
      return data.type == Type;                                                                                                \
    }                                                                                                                          \
    static T const &get(MMDB_entry_data_s const &data)                                                                         \
    {                                                                                                                          \
      return data.Member;                                                                                                      \
    }                                                                                                                          \
  }

REGISTER_GEOIP_DATA_TYPE(bool, MMDB_DATA_TYPE_BOOLEAN, boolean);
REGISTER_GEOIP_DATA_TYPE(std::uint16_t, MMDB_DATA_TYPE_UINT16, uint16);
REGISTER_GEOIP_DATA_TYPE(std::int32_t, MMDB_DATA_TYPE_INT32, int32);
REGISTER_GEOIP_DATA_TYPE(std::uint32_t, MMDB_DATA_TYPE_UINT32, uint32);
REGISTER_GEOIP_DATA_TYPE(std::uint64_t, MMDB_DATA_TYPE_UINT64, uint64);
REGISTER_GEOIP_DATA_TYPE(float, MMDB_DATA_TYPE_FLOAT, float_value);
REGISTER_GEOIP_DATA_TYPE(double, MMDB_DATA_TYPE_DOUBLE, double_value);

#undef REGISTER_GEOIP_DATA_TYPE

} // namespace geoip
