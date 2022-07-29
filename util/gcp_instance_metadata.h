/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/expected.h>
#include <util/ip_address.h>

#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

class GcpServiceAccount {
public:
  GcpServiceAccount(std::string name, std::string email, std::vector<std::string> scopes);

  std::string const &name() const { return name_; }
  std::string const &email() const { return email_; }
  std::vector<std::string> const &scopes() const { return scopes_; }

  void print() const;

private:
  std::string name_;
  std::string email_;
  std::vector<std::string> scopes_;
};

class GcpNetworkInterface {
public:
  using ip_address_t = std::variant<IPv4Address, IPv6Address>;

  GcpNetworkInterface(std::string vpc_id, std::string mac, ip_address_t ip, std::vector<IPv4Address> public_ips);

  std::string const &vpc_id() const { return vpc_id_; }
  std::string const &mac() const { return mac_; }

  // nullptr if not an ipv4
  IPv4Address const *ipv4() const { return std::holds_alternative<IPv4Address>(ip_) ? &std::get<IPv4Address>(ip_) : nullptr; }

  // nullptr if not an ipv6
  IPv6Address const *ipv6() const { return std::holds_alternative<IPv6Address>(ip_) ? &std::get<IPv6Address>(ip_) : nullptr; }

  std::vector<IPv4Address> const &public_ips() const { return public_ips_; }

  void print() const;

private:
  std::string vpc_id_;
  std::string mac_;
  ip_address_t ip_;
  std::vector<IPv4Address> public_ips_;
};

class GcpInstanceMetadata {
public:
  GcpInstanceMetadata(
      std::string cluster_name,
      std::string cluster_location,
      std::string image,
      std::string hostname,
      std::string name,
      std::int64_t id,
      std::string az,
      std::string role,
      std::string type,
      std::vector<GcpNetworkInterface> network_interfaces,
      std::vector<GcpServiceAccount> service_accounts);

  std::string const &cluster_name() const { return cluster_name_; }
  std::string const &cluster_location() const { return cluster_location_; }
  std::string const &image() const { return image_; }
  std::string const &hostname() const { return hostname_; }
  std::string const &name() const { return name_; }
  std::int64_t id() const { return id_; }
  std::string const &az() const { return az_; }
  std::string const &role() const { return role_; }
  std::string const &type() const { return type_; }
  std::vector<GcpNetworkInterface> const &network_interfaces() const { return network_interfaces_; }
  std::vector<GcpServiceAccount> const &service_accounts() const { return service_accounts_; }

  void print() const;

  static Expected<GcpInstanceMetadata, std::runtime_error> fetch(std::chrono::microseconds timeout);

private:
  std::string cluster_name_;
  std::string cluster_location_;
  std::string image_;
  std::string hostname_;
  std::string name_;
  std::int64_t id_;
  std::string az_;
  std::string role_;
  std::string type_;
  std::vector<GcpNetworkInterface> network_interfaces_;
  std::vector<GcpServiceAccount> service_accounts_;
};
