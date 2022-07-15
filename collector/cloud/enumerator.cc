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

#include <collector/cloud/enumerator.h>

#include <aws/ec2/model/DescribeNetworkInterfacesRequest.h>
#include <aws/ec2/model/DescribeNetworkInterfacesResponse.h>
#include <aws/ec2/model/DescribeRegionsRequest.h>
#include <aws/ec2/model/DescribeRegionsResponse.h>

#include <generated/flowmill/cloud_collector/index.h>
#include <generated/flowmill/ingest.wire_message.h>

#include <util/ip_address.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/resource_usage_reporter.h>
#include <util/stop_watch.h>

#include <array>
#include <type_traits>
#include <utility>

#include <cstdint>

namespace collector::cloud {

NetworkInterfacesEnumerator::NetworkInterfacesEnumerator(
    logging::Logger &log, flowmill::cloud_collector::Index &index, flowmill::ingest::Writer &writer)
    : index_(index), writer_(writer), log_(log)
{}

NetworkInterfacesEnumerator::~NetworkInterfacesEnumerator()
{
  free_handles();
}

void NetworkInterfacesEnumerator::set_handles(std::vector<flowmill::cloud_collector::handles::aws_network_interface> handles)
{
  free_handles();
  handles_ = std::move(handles);
}

void NetworkInterfacesEnumerator::free_handles()
{
  for (auto &handle : handles_) {
    handle.put(index_);
  }

  handles_.clear();
}

void translate_interfaces_to_spans(
    flowmill::cloud_collector::Index &index,
    Aws::Vector<Aws::EC2::Model::NetworkInterface> const &interfaces,
    std::vector<flowmill::cloud_collector::handles::aws_network_interface> &handles)
{
  for (auto const &interface : interfaces) {
    auto const &attachment = interface.GetAttachment();
    auto const &association = interface.GetAssociation();
    auto const &ip_owner_id = association.GetIpOwnerId();
    auto const &vpc_id = interface.GetVpcId();
    auto const &az = interface.GetAvailabilityZone();

    auto const &interface_id = interface.GetNetworkInterfaceId();
    auto const raw_interface_type = static_cast<std::uint16_t>(interface.GetInterfaceType());
    auto const &instance_id = attachment.GetInstanceId();
    auto const &instance_owner_id = attachment.GetInstanceOwnerId();
    auto const &public_dns_name = association.GetPublicDnsName();
    auto const &private_dns_name = interface.GetPrivateDnsName();
    auto const &description = interface.GetDescription();

    auto const add_entry = [&](IPv6Address const &ipv6) {
      auto handle = index.aws_network_interface.by_key({.ip = ipv6.as_int()});

      LOG::trace(
          "network_interface_info:"
          " ip={}"
          " ip_owner_id={}"
          " vpc_id={}"
          " az={}"
          " interface_id={} interface_type={} instance_id={} instance_owner_id={}"
          " public_dns_name={} private_dns_name={} description={}",
          ipv6,
          ip_owner_id,
          vpc_id,
          az,
          interface_id,
          raw_interface_type,
          instance_id,
          instance_owner_id,
          public_dns_name,
          private_dns_name,
          description);

      handle.network_interface_info(
          jb_blob{ip_owner_id},
          jb_blob{vpc_id},
          jb_blob{az},
          jb_blob{interface_id},
          raw_interface_type,
          jb_blob{instance_id},
          jb_blob{instance_owner_id},
          jb_blob{public_dns_name},
          jb_blob{private_dns_name},
          jb_blob{description});

      handles.emplace_back(handle.to_handle());
    };

    if (auto const public_ip = IPv4Address::parse(association.GetPublicIp().c_str())) {
      add_entry(public_ip->to_ipv6());
    }

    for (auto const &ipv4 : interface.GetPrivateIpAddresses()) {
      auto const private_ip = IPv4Address::parse(ipv4.GetPrivateIpAddress().c_str());

      if (private_ip) {
        add_entry(private_ip->to_ipv6());
      }
    }

    for (auto const &address : interface.GetIpv6Addresses()) {
      if (auto const ipv6 = IPv6Address::parse(address.GetIpv6Address().c_str())) {
        add_entry(*ipv6);
      }
    }
  }
}

void NetworkInterfacesEnumerator::handle_ec2_error(
    CollectorStatus status, Aws::Client::AWSError<Aws::EC2::EC2Errors> const &error)
{
  if (error.GetErrorType() == Aws::EC2::EC2Errors::THROTTLING) {
    log_.error("{} - AWS API call throttled: {}", status, error.GetMessage());
    return;
  }

  auto const http_status = static_cast<std::underlying_type_t<Aws::Http::HttpResponseCode>>(error.GetResponseCode());

  LOG::trace("reporting aws collector as unhealthy (status={} detail={})", status, http_status);
  writer_.collector_health(integer_value(status), http_status);

  if (http_status >= 400 && http_status < 500) {
    log_.error(
        "{} -  API call failed with http status {}. Double check that AWS credentials are"
        " properly set up for this pod. Check Flowmill setup instructions for more"
        " information. Error message from AWS: {}",
        status,
        http_status,
        error.GetMessage());
  } else {
    log_.error(
        "{} -  API call failed with http status {}. Error message from AWS: {}", status, http_status, error.GetMessage());
  }
}

scheduling::JobFollowUp NetworkInterfacesEnumerator::enumerate()
{
  ResourceUsageReporter::report(writer_);

  auto const regions_response = ec2_.DescribeRegions({});
  if (!regions_response.IsSuccess()) {
    handle_ec2_error(CollectorStatus::aws_describe_regions_error, regions_response.GetError());
    return scheduling::JobFollowUp::backoff;
  }

  std::vector<flowmill::cloud_collector::handles::aws_network_interface> handles;
  auto result = scheduling::JobFollowUp::ok;

  LOG::trace("starting AWS network interfaces enumeration");
  StopWatch<> watch;
  for (auto const &region : regions_response.GetResult().GetRegions()) {
    LOG::trace("enumerating network interfaces in region '{}'", region.GetRegionName());

    Aws::Client::ClientConfiguration client_config;
    client_config.region = region.GetRegionName();

    Aws::EC2::EC2Client client(client_config);

    auto const interfaces_response = client.DescribeNetworkInterfaces({});

    if (!interfaces_response.IsSuccess()) {
      handle_ec2_error(CollectorStatus::aws_describe_network_interfaces_error, interfaces_response.GetError());
      result = scheduling::JobFollowUp::backoff;
      continue;
    }

    auto const &interfaces = interfaces_response.GetResult().GetNetworkInterfaces();

    LOG::trace("found {} network interfaces in region '{}'", interfaces.size(), region.GetRegionName());

    translate_interfaces_to_spans(index_, interfaces, handles);
  }
  LOG::trace("finished AWS network interfaces enumeration after {}", watch.elapsed<std::chrono::milliseconds>());

  set_handles(std::move(handles));

  LOG::trace("network interface live span count: {}", handles_.size());

  if (result == scheduling::JobFollowUp::ok) {
    LOG::trace("reporting aws collector as healthy");
    writer_.collector_health(integer_value(CollectorStatus::healthy), 0);
  }

  return result;
}

} // namespace collector::cloud
