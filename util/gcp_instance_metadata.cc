// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/gcp_instance_metadata.h>

#include <common/cloud_platform.h>
#include <util/json.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/restful.h>
#include <util/string.h>

#include <optional>
#include <utility>

#include <cassert>

namespace {

// metadata fetching described in TODO
static constexpr std::string_view METADATA_FLAVOR_HEADER = "Metadata-Flavor: Google";
static constexpr std::string_view METADATA_URL = "http://metadata.google.internal/"
                                                 "computeMetadata/v1/instance/"
                                                 "?recursive=true";

static constexpr auto METADATA_QUERY_TIMEOUT = 1s;

} // namespace

GcpServiceAccount::GcpServiceAccount(std::string name, std::string email, std::vector<std::string> scopes)
    : name_(std::move(name)), email_(std::move(email)), scopes_(std::move(scopes))
{}

void GcpServiceAccount::print() const
{
  LOG::debug_in(CloudPlatform::gcp, "    name: {}", name_);
  LOG::debug_in(CloudPlatform::gcp, "    email: {}", email_);
  LOG::debug_in(CloudPlatform::gcp, "    scopes: {}", scopes_.size());
  for (std::size_t i = 0; i < scopes_.size(); ++i) {
    LOG::debug_in(CloudPlatform::gcp, "    - {}", scopes_[i]);
  }
}

GcpNetworkInterface::GcpNetworkInterface(
    std::string vpc_id, std::string mac, ip_address_t ip, std::vector<IPv4Address> public_ips)
    : vpc_id_(std::move(vpc_id)), mac_(std::move(mac)), ip_(std::move(ip)), public_ips_(std::move(public_ips))
{}

void GcpNetworkInterface::print() const
{
  LOG::debug_in(CloudPlatform::gcp, "    vpc id: {}", vpc_id_);
  LOG::debug_in(CloudPlatform::gcp, "    mac: {}", mac_);
  LOG::debug_in(CloudPlatform::gcp, "    ip: {}", ip_);
  LOG::debug_in(CloudPlatform::gcp, "    public ips: {}", public_ips_.size());
  for (std::size_t i = 0; i < public_ips_.size(); ++i) {
    LOG::debug_in(CloudPlatform::gcp, "    - {}", public_ips_[i]);
  }
}

GcpInstanceMetadata::GcpInstanceMetadata(
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
    std::vector<GcpServiceAccount> service_accounts)
    : cluster_name_(std::move(cluster_name)),
      cluster_location_(std::move(cluster_location)),
      image_(std::move(image)),
      hostname_(std::move(hostname)),
      name_(std::move(name)),
      id_(id),
      az_(std::move(az)),
      role_(std::move(role)),
      type_(std::move(type)),
      network_interfaces_(std::move(network_interfaces)),
      service_accounts_(std::move(service_accounts))
{}

void GcpInstanceMetadata::print() const
{
  LOG::debug_in(CloudPlatform::gcp, "GCP metadata:");
  LOG::debug_in(CloudPlatform::gcp, "  cluster name: {}", cluster_name_);
  LOG::debug_in(CloudPlatform::gcp, "  cluster location: {}", cluster_location_);
  LOG::debug_in(CloudPlatform::gcp, "  instance image: {}", image_);
  LOG::debug_in(CloudPlatform::gcp, "  hostname: {}", hostname_);
  LOG::debug_in(CloudPlatform::gcp, "  name: {}", name_);
  LOG::debug_in(CloudPlatform::gcp, "  id: {}", id_);
  LOG::debug_in(CloudPlatform::gcp, "  az: {}", az_);
  LOG::debug_in(CloudPlatform::gcp, "  role: {}", role_);
  LOG::debug_in(CloudPlatform::gcp, "  type: {}", type_);

  LOG::debug_in(CloudPlatform::gcp, "  network interfaces: {}", network_interfaces_.size());
  for (std::size_t i = 0; i < network_interfaces_.size(); ++i) {
    LOG::debug_in(CloudPlatform::gcp, "  - network interface: {}", i);
    network_interfaces_[i].print();
  }

  LOG::debug_in(CloudPlatform::gcp, "  service accounts: {}", service_accounts_.size());
  for (std::size_t i = 0; i < service_accounts_.size(); ++i) {
    LOG::debug_in(CloudPlatform::gcp, "  - service account: {}", i);
    service_accounts_[i].print();
  }
}

Expected<GcpInstanceMetadata, std::runtime_error> GcpInstanceMetadata::fetch(std::chrono::microseconds timeout)
{
  RestfulFetcher fetcher({METADATA_FLAVOR_HEADER});

  auto const metadata = fetcher.sync_fetch<nlohmann::json>(
      "Google Cloud Platform instance metadata",
      std::string(METADATA_URL),
      [](std::string_view json) -> Expected<nlohmann::json, std::runtime_error> {
        try {
          return nlohmann::json::parse(json);
        } catch (std::exception const &e) {
          return {unexpected, e.what()};
        }
      },
      METADATA_QUERY_TIMEOUT);

  if (!metadata) {
    return {unexpected, std::move(metadata.error())};
  }

  std::vector<GcpNetworkInterface> network_interfaces;
  if (auto const interfaces = follow_path(*metadata, "networkInterfaces"); interfaces && interfaces->is_array()) {
    for (auto const &interface : *interfaces) {
      std::vector<IPv4Address> public_ips;
      if (auto const access_configs = follow_path(interface, "accessConfigs"); access_configs && access_configs->is_array()) {
        for (auto const &access_config : *access_configs) {
          if (auto public_ip = IPv4Address::parse(get_zstring_view(access_config, "externalIp").data())) {
            public_ips.push_back(std::move(*public_ip));
          }
        }
      }

      // TODO: double-check how ipv6 manifests itself
      std::optional<GcpNetworkInterface::ip_address_t> ip_address;
      {
        auto const ip = get_zstring_view(interface, "ip");
        if (auto ipv4 = IPv4Address::parse(ip.data())) {
          ip_address.emplace(std::in_place_type<IPv4Address>, std::move(ipv4.value()));
        } else if (auto ipv6 = IPv6Address::parse(ip.data())) {
          assert(public_ips.empty());
          ip_address.emplace(std::in_place_type<IPv6Address>, std::move(ipv6.value()));
        } else {
          continue;
        }
      }

      network_interfaces.emplace_back(
          std::string(last_token(get_string_view(interface, "network"), '/')),
          std::string(get_string_view(interface, "mac")),
          std::move(*ip_address),
          std::move(public_ips));
    }
  }

  std::vector<GcpServiceAccount> service_accounts;
  if (auto const accounts = follow_path(*metadata, "serviceAccounts"); accounts && accounts->is_object()) {
    for (auto const &account_item : accounts->items()) {
      std::vector<std::string> scopes;
      auto const &account = account_item.value();
      if (auto const account_scopes = follow_path(account, "scopes"); account_scopes && account_scopes->is_array()) {
        for (auto const &scope : *account_scopes) {
          if (auto const scope_string = try_get_string(scope)) {
            scopes.push_back(*scope_string);
          }
        }
      }

      service_accounts.emplace_back(
          std::string(get_string_view(account_item.key())), std::string(get_string_view(account, "email")), std::move(scopes));
    }
  }

  auto const name = get_string_view(*metadata, "name");
  auto const id = try_get_int(*metadata, "id");

  return GcpInstanceMetadata{
      std::string(get_string_view(*metadata, "attributes", "cluster-name")),
      std::string(get_string_view(*metadata, "attributes", "cluster-location")),
      std::string(get_string_view(*metadata, "image")),
      std::string(get_string_view(*metadata, "hostname")),
      std::string(name),
      id.value_or(0),
      std::string(last_token(get_string_view(*metadata, "zone"), '/')),
      // is this the best attribute to use as role?
      service_accounts.empty() ? std::string(name) : service_accounts.front().name(),
      std::string(last_token(get_string_view(*metadata, "machineType"), '/')),
      std::move(network_interfaces),
      std::move(service_accounts)

  };
}
