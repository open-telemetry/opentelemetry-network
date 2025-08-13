// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "aws_network_interface_span.h"

#include <reducer/constants.h>

#include <util/log.h>

#include <absl/strings/match.h>

namespace reducer::collector::cloud {

constexpr std::string_view amazon_owner_prefix = "amazon-";
constexpr std::string_view aws_nat_gateway_prefix = "Interface for ";

AwsNetworkInterfaceSpan::AwsNetworkInterfaceSpan() {}

AwsNetworkInterfaceSpan::~AwsNetworkInterfaceSpan() {}

void AwsNetworkInterfaceSpan::network_interface_info(
    ::ebpf_net::ingest::weak_refs::aws_network_interface span_ref, u64 timestamp, jsrv_ingest__network_interface_info *msg)
{
  LOG::trace_in(
      std::make_tuple(NodeResolutionType::AWS, ClientType::cloud),
      "ingest::AwsNetworkInterfaceSpan::network_interface_info: incoming"
      " ip_owner_id={} vpc_id={} az={}"
      " interface_id={} interface_type={} instance_id={} instance_owner_id={}"
      " public_dns_name={} private_dns_name={} description='{}'",
      msg->ip_owner_id,
      msg->vpc_id,
      msg->az,
      msg->interface_id,
      msg->interface_type,
      msg->instance_id,
      msg->instance_owner_id,
      msg->public_dns_name,
      msg->private_dns_name,
      msg->interface_description);

  // AWS instances always have either "Attachement.IpOwnerId" or
  // "Attachment.InstanceOwnerId" starting with "amazon-aws"
  bool const owned_by_aws = msg->ip_owner_id.string_view().starts_with(amazon_owner_prefix) ||
                            msg->instance_owner_id.string_view().starts_with(amazon_owner_prefix);

  if (!owned_by_aws) {
    LOG::trace_in(
        std::make_tuple(NodeResolutionType::AWS, ClientType::cloud),
        "ingest::AwsNetworkInterfaceSpan::network_interface_info: not owned by "
        "aws"
        " ip_owner_id={} vpc_id={} az={}"
        " interface_id={} interface_type={} instance_id={} instance_owner_id={}"
        " public_dns_name={} private_dns_name={} description='{}'",
        msg->ip_owner_id,
        msg->vpc_id,
        msg->az,
        msg->interface_id,
        msg->interface_type,
        msg->instance_id,
        msg->instance_owner_id,
        msg->public_dns_name,
        msg->private_dns_name,
        msg->interface_description);
    return;
  }

  std::string_view aws_role = msg->interface_description;
  auto const aws_az = msg->az.string_view();
  auto const aws_id = msg->interface_id.string_view();

  // cleans up nat gateway description
  if (aws_role.starts_with(aws_nat_gateway_prefix)) {
    assert(aws_role.size() >= aws_nat_gateway_prefix.size());
    aws_role.remove_prefix(aws_nat_gateway_prefix.size());
    LOG::trace_in(
        std::make_tuple(NodeResolutionType::AWS, ClientType::cloud),
        "ingest::AwsNetworkInterfaceSpan::network_interface_info:"
        " nat_gateway role truncated (prefix='{}' role='{}')",
        aws_nat_gateway_prefix,
        aws_role);
  } else {
    LOG::trace_in(
        std::make_tuple(NodeResolutionType::AWS, ClientType::cloud),
        "ingest::AwsNetworkInterfaceSpan::network_interface_info:"
        " nat_gateway role doesn't need cleaning (prefix='{}' role='{}')",
        aws_nat_gateway_prefix,
        aws_role);
  }

  bool const can_enrich = !aws_role.empty() && !aws_az.empty();

  if (can_enrich) {
    span_ref.aws_enrichment(jb_blob{aws_role}, jb_blob{aws_az}, jb_blob{aws_id});
  } else {
    LOG::warn(
        "ingest::AwsNetworkInterfaceSpan::network_interface_info:"
        " refusing to enrich using AWS metadata:"
        " ip_owner_id={} vpc_id={} az={}"
        " interface_id={} interface_type={}"
        " instance_id={} instance_owner_id={}"
        " public_dns_name={} private_dns_name={} description='{}'"
        " - enrichment[can={}]: role='{}' az='{}'",
        msg->ip_owner_id,
        msg->vpc_id,
        msg->az,
        msg->interface_id,
        static_cast<std::uint16_t>(msg->interface_type),
        msg->instance_id,
        msg->instance_owner_id,
        msg->public_dns_name,
        msg->private_dns_name,
        msg->interface_description,
        can_enrich,
        aws_role,
        aws_az);
  }
}

void AwsNetworkInterfaceSpan::network_interface_info_deprecated(
    ::ebpf_net::ingest::weak_refs::aws_network_interface span_ref,
    u64 timestamp,
    jsrv_ingest__network_interface_info_deprecated *msg)
{
  jsrv_ingest__network_interface_info message;
  message.ip_owner_id = render_array_to_string_view(msg->ip_owner_id);
  message.vpc_id = render_array_to_string_view(msg->vpc_id);
  message.az = render_array_to_string_view(msg->az);
  message.interface_id = msg->interface_id;
  message.interface_type = msg->interface_type;
  message.instance_id = msg->instance_id;
  message.instance_owner_id = msg->instance_owner_id;
  message.public_dns_name = msg->public_dns_name;
  message.private_dns_name = msg->private_dns_name;
  message.interface_description = msg->interface_description;
  network_interface_info(span_ref, timestamp, &message);
}

} // namespace reducer::collector::cloud
