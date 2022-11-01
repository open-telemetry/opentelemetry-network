/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <maxminddb.h>

#include <initializer_list>
#include <new>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include <cassert>
#include <cstdint>

#include "geoip.inl"

/**
 * RAII-style user friendly API to query GeoIP databases.
 *
 * Example:
 *
 *  using namespace geoip;
 *
 *  database db("path/to/geoip-db.mmdb");
 *
 *  auto entry = db.lookup("1.2.3.4");
 *  if (!entry) {
 *    std::cout << "no entry found for ip 1.2.3.4\n";
 *    return;
 *  }
 *
 *  std::cout << entry.get_as<std::string_view>("autonomous_system_organization") << '\n';
 *
 *  std::uint32_t asn;
 *  if (entry.try_get_as(asn, "autonomous_system_number") {
 *    std::cout << asn << '\n';
 *  }
 *
 *  std::cout << well_known_data::autonomous_system_organization(entry) << '\n';
 */
namespace geoip {

/*
 * Encapsulates a unit of data retrieved from the GeoIP database.
 *
 * struct api:
 *  https://github.com/maxmind/libmaxminddb/blob/master/doc/libmaxminddb.md#mmdb_entry_data_s
 *
 * data types:
 *  https://github.com/maxmind/libmaxminddb/blob/master/doc/libmaxminddb.md#data-type-macros
 */
struct data_unit {
  using data_t = MMDB_entry_data_s;

  /* implicit */
  data_unit(data_t const &data) : data_(data) {}

  /**
   * Tells whether this data unit represents a string of bytes.
   */
  bool is_bytes() const { return data_.type == MMDB_DATA_TYPE_BYTES; }

  /**
   * Encloses the byte string in this data unit into the `out` string view if it represents a
   * string of bytes, otherwise returns `false` and leaves `out` untouched.
   */
  bool try_bytes(std::string_view &out) const
  {
    assert(valid());

    if (!is_bytes()) {
      return false;
    }

    out = {reinterpret_cast<char const *>(data_.bytes), data_.data_size};
    return true;
  }

  /**
   * Returns a string view of the byte string in this data unit.
   * No type checking is performed so if the data unit doesn't represent a byte string,
   * the contents of the string view are undefined.
   */
  std::string_view bytes() const
  {
    assert(valid());
    assert(is_bytes());

    return {reinterpret_cast<char const *>(data_.bytes), data_.data_size};
  }

  /**
   * Tells whether this data unit represents a UTF-8 string.
   */
  bool is_utf8() const { return data_.type == MMDB_DATA_TYPE_UTF8_STRING; }

  /**
   * Encloses the UTF-8 string in this data unit into the `out` string view if it represents a
   * UTF-8 string, otherwise returns `false` and leaves `out` untouched.
   */
  bool try_utf8(std::string_view &out) const
  {
    assert(valid());

    if (!is_utf8()) {
      return false;
    }

    out = {data_.utf8_string, data_.data_size};
    return true;
  }

  /**
   * Returns a string view of the UTF-8 string in this data unit.
   * No type checking is performed so if the data unit doesn't represent a UTF-8 string,
   * the contents of the string view are undefined.
   */
  std::string_view utf8() const
  {
    assert(valid());
    assert(is_utf8());

    return {data_.utf8_string, data_.data_size};
  }

  /**
   * Returns true if `T` can represent this data unit's data type.
   */
  template <typename T> bool is() const { return data_type<std::decay_t<T>>::same(data_); }

  /**
   * Converts this data unit's data to `T` and assigns it to `out` if `T` can represent this
   * data unit's data type. Otherwise returns `false` and leaves `out` untouched.
   */
  template <typename T> bool try_to(T &out) const
  {
    assert(valid());

    if (!is<T>()) {
      return false;
    }

    out = data_type<std::decay_t<T>>::get(data_);
    return true;
  }

  /**
   * Converts this data unit's data to `T` and returns it.
   * No type checking is performed so if the data unit can't be represented by `T`, the result is
   * undefined.
   */
  template <typename T> auto to() const
  {
    assert(valid());
    assert(is<T>());

    return data_type<std::decay_t<T>>::get(data_);
  }

  /**
   * Tells whether there's any valid data represented by this data unit.
   */
  bool valid() const { return data_.has_data; }

  /**
   * Tells whether there's any valid data represented by this data unit.
   */
  explicit operator bool() const { return valid(); }

  /**
   * Tells whether this data unit doesn't have any valid data.
   */
  bool operator!() const { return !valid(); }

private:
  data_t data_;
};

/**
 * Encapsulates the result of a lookup into the GeoIP database.
 *
 * data lookup api:
 *  https://github.com/maxmind/libmaxminddb/blob/master/doc/libmaxminddb.md#data-lookup-functions
 */
struct address_entry {
  using entry_t = MMDB_lookup_result_s;

  /* implicit */
  address_entry(entry_t const &entry) : entry_(entry) {}

  /**
   * Retrieves a data unit for the given path.
   *
   * E.g.:
   *
   *    auto data = entry.get("autonomous_system_organization);
   */
  template <typename... Key> data_unit get(Key const *...keys)
  {
    char const *const path[] = {keys..., nullptr};
    return get_impl(path);
  }

  /**
   * Retrieves a data unit for the given path, then converts it to `T`, assigns it to `out` and
   * returns true..
   *
   * If the path doesn't contain any data or if the data can't be converted to `T`,
   * returns `false` and leaves `out` untouched.
   */
  template <typename T, typename... Key> bool try_get_as(T &out, Key const *...keys)
  {
    auto data = get(keys...);

    return data && data.template try_to<T>(out);
  }

  /**
   * Retrieves a data unit for the given path, then converts it to `T` and returns it.
   * No type checking is performed so if the path doesn't have any data, or if the data can't be
   * represented by `T`, the result is undefined.
   */
  template <typename T, typename... Key> T get_as(Key const *...keys)
  {
    auto data = get(keys...);

    assert(data.valid());

    return data.template to<T>();
  }

  /**
   * Tells whether this entry is valid and can be used to retrieve data from.
   */
  bool valid() const { return entry_.found_entry; }

  /**
   * Tells whether this entry is valid and can be used to retrieve data from.
   */
  explicit operator bool() const { return valid(); }

  /**
   * Tells whether this entry is invalid and can't be used to retrieve data from.
   */
  bool operator!() const { return !valid(); }

private:
  entry_t entry_;

  data_unit get_impl(char const *const *path);
};

/**
 * Helper for retrieval of well known data entries from a GeoIP database.
 */
struct well_known_data {
  struct keys {
    static constexpr auto const autonomous_system_number = "autonomous_system_number";
    static constexpr auto const autonomous_system_organization = "autonomous_system_organization";
  };

  /**
   * Retrieves the autonomous system number from the given entry if it is available. Returns
   * `true` on success. If the data is not available, returns `false` and leaves `out` untouched.
   */
  static bool try_autonomous_system_number(std::uint32_t &out, address_entry &entry)
  {
    return entry.try_get_as(out, keys::autonomous_system_number);
  }

  /**
   * Retrieves the autonomous system number from the given entry.
   * No checks are performed so if the data is not available, the result is undefined.
   */
  static auto autonomous_system_number(address_entry &entry)
  {
    return entry.get_as<std::uint32_t>(keys::autonomous_system_number);
  }

  /**
   * Retrieves the autonomous system organization from the given entry if it is available. Returns
   * `true` on success. If the data is not available, returns `false` and leaves `out` untouched.
   */
  template <typename String> static bool try_autonomous_system_organization(String &out, address_entry &entry)
  {
    return entry.try_get_as(out, keys::autonomous_system_organization);
  }

  /**
   * Retrieves the autonomous system organization from the given entry.
   * No checks are performed so if the data is not available, the result is undefined.
   */
  static auto autonomous_system_organization(address_entry &entry)
  {
    return entry.get_as<std::string>(keys::autonomous_system_organization);
  }

private:
  data_unit data_;
};

/*
 * Encapsulates a GeoIP database.
 *
 * api:
 *  https://github.com/maxmind/libmaxminddb/blob/master/doc/libmaxminddb.md
 */
struct database {
  enum class open_mode : std::uint32_t {
    /**
     * Uses the default mode from `libmaxminddb` to open the database.
     */
    standard = 0,

    /**
     * Uses mmap to open the database.
     */
    mmap = MMDB_MODE_MMAP
  };

  /**
   * Opens the GeoIP database from the given file.
   *
   * The way the database is opened can be specified through the `mode` flag.
   *
   * Throws `std::runtime_error` if unable to open.
   */
  explicit database(char const *db_path, open_mode mode = open_mode::mmap);

  /**
   * Opens the GeoIP database from the given file.
   *
   * The way the database is opened can be specified through the `mode` flag.
   *
   * This is the non throwing version of the constructor. Use the boolean operator
   * to check if the database has been loaded.
   */
  explicit database(std::nothrow_t, char const *db_path, open_mode mode = open_mode::mmap);

  /**
   * Opens the GeoIP database from the given files. Tries each file in succession until one
   * of them succeeds.
   *
   * The way the database is opened can be specified through the `mode` flag.
   *
   * Throws `std::runtime_error` if unable to open.
   */
  explicit database(std::initializer_list<char const *> db_path, open_mode mode = open_mode::mmap);

  /**
   * Opens the GeoIP database from the given files. Tries each file in succession until one
   * of them succeeds.
   *
   * The way the database is opened can be specified through the `mode` flag.
   *
   * This is the non throwing version of the constructor. Use the boolean operator
   * to check if the database has been loaded.
   */
  explicit database(std::nothrow_t, std::initializer_list<char const *> db_path, open_mode mode = open_mode::mmap);

  /**
   * Closes the database file and destroys this object.
   */
  ~database();

  database(database &&) = default;
  database(database const &) = delete;

  database &operator=(database &&) = default;
  database &operator=(database const &) = delete;

  /**
   * Uses the given raw address to lookup an entry in the GeoIP database.
   */
  address_entry lookup(sockaddr const *address);

  /**
   * Uses the given raw IPv4 address to lookup an entry in the GeoIP database.
   */
  address_entry lookup(in_addr const *ip_address);

  /**
   * Uses the given raw IPv6 address to lookup an entry in the GeoIP database.
   */
  address_entry lookup(in6_addr const *ip_address);

  /**
   * Uses the given raw IPv4 address to lookup an entry in the GeoIP database.
   */
  address_entry lookup(sockaddr_in const *address) { return lookup(reinterpret_cast<sockaddr const *>(address)); }

  /**
   * Uses the given raw IPv6 address to lookup an entry in the GeoIP database.
   */
  address_entry lookup(sockaddr_in6 const *address) { return lookup(reinterpret_cast<sockaddr const *>(address)); }

  /**
   * Uses the string representation of the address to lookup an entry in the GeoIP database.
   *
   * The underlying implementation will resolve the string to a raw address structure before
   * performing the lookup, so if you have the raw address already, use the alternative overload.
   */
  address_entry lookup(char const *ip);

  /**
   * Tells whether the database was successfully loaded or not.
   */
  explicit operator bool() const { return db_.has_value(); }

  /**
   * Tells whether the database failed or succeeded to load.
   */
  bool operator!() const { return !db_.has_value(); }

  /**
   * Default local path for the Autonomous Systems database.
   */
  static constexpr char const *local_asn_path = "GeoLite2-ASN.mmdb";

  /**
   * Default global path for the Autonomous Systems database.
   */
  static constexpr char const *global_asn_path = "/usr/share/GeoIP/GeoLite2-ASN.mmdb";

private:
  std::optional<::MMDB_s> db_ = {};
};

} // namespace geoip
