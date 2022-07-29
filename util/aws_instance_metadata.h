/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/expected.h>

#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

struct AwsMetadataValue {
  AwsMetadataValue() : retcode_(-1) {}
  AwsMetadataValue(std::string v, int rc) : value_(std::move(v)), retcode_(rc) {}
  friend std::ostream &operator<<(std::ostream &os, const AwsMetadataValue &val);

  std::string_view value() const;

  void set(std::string_view value, int retcode = 0)
  {
    value_ = value;
    retcode_ = 0;
  }

  bool valid() const { return retcode_ == 0; }

  explicit operator bool() const { return valid(); }
  bool operator!() const { return !valid(); }

private:
  std::string value_;
  int retcode_;
};

class AwsNetworkInterface {
public:
  using FetchResult = std::map<std::string, AwsMetadataValue>;

  /**
   * c'tor
   */
  AwsNetworkInterface(std::string interface_id);

  /*
   * Sets vpc_id_, private_ipv4s_, public_ipv4s_, and ipv6s_.
   * @param info: the result of AwsMetadata::fetch()
   */
  void set_info(FetchResult const &info);

  /*
   * Sets mapped_ipv4s_.
   * @param info: the result of AwsMetadata::fetch()
   */
  void set_mapped_ipv4s(FetchResult const &info);

  /**
   * Prints information for this interface
   */
  void print_interface() const;

  /* accessors */
  std::string const &id() const { return interface_id_; };
  std::string const &vpc_id() const { return vpc_id_; };
  std::set<std::string> const &private_ipv4s() const { return private_ipv4s_; };
  std::vector<std::string> const &public_ipv4s() const { return public_ipv4s_; };
  std::vector<std::string> const &ipv6s() const { return ipv6s_; };
  std::map<std::string, std::string> const &mapped_ipv4s() const { return mapped_ipv4s_; };

private:
  /*
   * A helper function that fills a vector with lines from a
   * multiline curl response.
   * @param info: One of the responses recieved from AwsMetadata::fetch()
   * @param dest: The vector to be filled.
   *
   * This is used specifically to fill {public/private}_ipv4s_ and ipv6s_.
   */
  void set_multiline_info(AwsMetadataValue const &info, std::vector<std::string> &dest);

  std::string interface_id_;
  std::string vpc_id_;
  std::set<std::string> private_ipv4s_;
  std::vector<std::string> public_ipv4s_;
  std::vector<std::string> ipv6s_;
  std::map<std::string, std::string> mapped_ipv4s_;
};

class AwsMetadata {
public:
  using FetchResult = AwsNetworkInterface::FetchResult;

  /**
   * Prints instance metadata without interface info
   */
  void print_instance_metadata() const;

  /**
   * Prints interface info
   */
  void print_interfaces() const;

  /* accessors */
  AwsMetadataValue const &id() const { return id_; };
  AwsMetadataValue const &az() const { return az_; };
  AwsMetadataValue const &iam_role() const { return iam_role_; };
  AwsMetadataValue const &type() const { return type_; };
  AwsMetadataValue const &account_id() const { return account_id_; };

  const std::vector<AwsNetworkInterface> &network_interfaces() const { return network_interfaces_; };

  static Expected<AwsMetadata, std::runtime_error> fetch(std::chrono::microseconds timeout);

private:
  /**
   * c'tor
   * @param timeout_usec: the timeout any fetches that will be performed.
   */
  AwsMetadata(std::chrono::microseconds timeout);

  /**
   * Calls fetch for some meta-data fields
   *
   * Returns true on success or false on failure.
   */
  bool fetch_aws_instance_metadata();

  /**
   * Fetches data in parallel with a timeout of timeout_usec_
   *
   * @assumes curl_global_init() was called!
   *
   * @param keys_to_endpoints: a map of <key> to <endpoint>
   * @result: a map of <key> to <result> of a curl to the corresponding endpoint
   */
  FetchResult fetch(std::map<std::string, std::string> keys_to_endpoints);

  std::chrono::microseconds timeout_;
  AwsMetadataValue id_;
  AwsMetadataValue az_;
  AwsMetadataValue iam_role_;
  AwsMetadataValue type_;
  AwsMetadataValue account_id_;
  std::vector<AwsNetworkInterface> network_interfaces_;
};
