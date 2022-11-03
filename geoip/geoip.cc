// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "geoip.h"

#include <cstring>

namespace geoip {

///////////////////
// address_entry //
///////////////////

data_unit address_entry::get_impl(char const *const *path)
{
  MMDB_entry_data_s data;
  auto result = ::MMDB_aget_value(&entry_.entry, &data, path);

  if (result != MMDB_SUCCESS) {
    data.has_data = false;
  }

  return data_unit(data);
}

//////////////
// database //
//////////////

database::database(std::nothrow_t, std::initializer_list<char const *> db_path, open_mode mode)
{
  auto flags = static_cast<std::uint32_t>(mode);

  for (auto const path : db_path) {
    if (!path) {
      continue;
    }

    auto result = ::MMDB_open(path, flags, &db_.emplace());

    if (result == MMDB_SUCCESS) {
      break;
    }

    db_.reset();
  }
}

database::database(std::initializer_list<char const *> db_path, open_mode mode)
{
  auto flags = static_cast<std::uint32_t>(mode);

  if (!db_path.size()) {
    throw std::runtime_error("no geoip database paths given");
  }

  int error = MMDB_SUCCESS;
  for (auto const path : db_path) {
    if (!path) {
      continue;
    }

    auto result = ::MMDB_open(path, flags, &db_.emplace());

    if (result == MMDB_SUCCESS) {
      break;
    } else {
      error = result;
    }

    db_.reset();
  }

  if (!db_.has_value()) {
    assert(error != MMDB_SUCCESS);
    throw std::runtime_error(::MMDB_strerror(error));
  }
}

database::database(char const *db_path, open_mode mode) : database({db_path}, mode) {}

database::database(std::nothrow_t, char const *db_path, open_mode mode) : database(std::nothrow, {db_path}, mode) {}

database::~database()
{
  if (db_.has_value()) {
    ::MMDB_close(&*db_);
  }
  db_.reset();
}

address_entry database::lookup(sockaddr const *address)
{
  int error = MMDB_SUCCESS;
  auto entry = ::MMDB_lookup_sockaddr(&*db_, address, &error);

  if (error != MMDB_SUCCESS) {
    entry.found_entry = false;
  }

  return address_entry(entry);
}

address_entry database::lookup(in_addr const *ip_address)
{
  sockaddr_in address{};
  address.sin_family = AF_INET;
  std::memcpy(&address.sin_addr, ip_address, sizeof(*ip_address));
  return lookup(&address);
}

address_entry database::lookup(in6_addr const *ip_address)
{
  sockaddr_in6 address{};
  address.sin6_family = AF_INET6;
  std::memcpy(&address.sin6_addr, ip_address, sizeof(*ip_address));
  return lookup(&address);
}

address_entry database::lookup(char const *ip)
{
  int gai_error = 0;
  int mmdb_error = MMDB_SUCCESS;
  auto entry = ::MMDB_lookup_string(&*db_, ip, &gai_error, &mmdb_error);

  if (gai_error || mmdb_error != MMDB_SUCCESS) {
    entry.found_entry = false;
  }

  return address_entry(entry);
}

} // namespace geoip
