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

#include "spdlog/common.h"
#include <collector/kernel/dns_requests.h>
#include <platform/platform.h>
#include <util/log.h>

/**
 * DNS requests key hash function
 *
 * @param[in] k The dns request key to hash
 * @return the hash of the dns request key
 */
size_t DnsRequests::dns_request_key_hash::operator()(const dns_request_key &k) const noexcept
{
  return std::hash<uint16_t>{}(k.qid) ^ std::hash<uint16_t>{}(k.type) ^ std::hash<std::string>{}(k.name);
}

/**
 * DNS requests key comparison function
 *
 * @param[in] k The first dns request key to compare
 * @param[in] k2 The second dns request key to compare
 * @return true if the keys are equal, false otherwise
 */
bool DnsRequests::dns_request_key_equal_to::operator()(const dns_request_key &k, const dns_request_key &k2) const noexcept
{
  return k.qid == k2.qid && k.type == k2.type && k.name == k2.name;
}

/**
 * Add a DNS Request to the data structure
 *
 * @param[in] key Key of the DNS request
 * @param[in] value Value of the DNS request
 */
void DnsRequests::add(const dns_request_key &key, const dns_request_value &value)
{
  auto iter = dns_requests_.insert(dns_requests_.end(), std::make_pair(key, value));
  dns_requests_by_key_.insert(std::make_pair(key, iter));
  dns_requests_by_sock_.insert(std::make_pair(value.sk, iter));
}

/**
 * Return a list of DNS Requests that correspond to a particular key
 *
 * @param[in] key Key of the DNS Request to look up
 * @param[out] out The list of matching dns request key/value pairs
 */
void DnsRequests::lookup(const dns_request_key &key, std::list<Request> &out)
{
  auto key_er = dns_requests_by_key_.equal_range(key);
  for (auto eriter = key_er.first; eriter != key_er.second; eriter++) {
    out.push_back(eriter->second);
  }
}

/**
 * Return a list of DNS Requests that are older than a particular timestamp
 *
 * @param[in] timestamp_ns The timestamp to get requests older than
 * @param[out] out The list of matching dns request key/value pairs
 */

void DnsRequests::lookup_older_than(u64 timestamp_ns, std::list<DnsRequests::Request> &out)
{
  for (auto iter = dns_requests_.begin(); iter != dns_requests_.end(); iter++) {
    if (iter->second.timestamp_ns >= timestamp_ns) {
      break;
    }

    out.push_back(iter);
  }
}

/**
 * Return a list of DNS Requests that match a particular socket
 *
 * @param[in] sk The kernel `struct sock*` pointer of the socket to look up
 * @param[out] out The list of matching dns request key/value pairs
 */
void DnsRequests::lookup_socket(u64 sk, std::list<DnsRequests::Request> &out)
{
  auto sk_er = dns_requests_by_sock_.equal_range(sk);
  for (auto eriter = sk_er.first; eriter != sk_er.second; eriter++) {
    out.push_back(eriter->second);
  }
}

/**
 * Removes a specific DNS Request
 *
 * @param[in] key The DNS Request to remove, as returned by lookup* functions
 */
void DnsRequests::remove(const Request &req)
{
  auto key_er = dns_requests_by_key_.equal_range(req->first);
  for (auto eriter = key_er.first; eriter != key_er.second; eriter++) {
    if (eriter->second == req) {
      dns_requests_by_key_.erase(eriter);
      break;
    }
  }

  auto sk_er = dns_requests_by_sock_.equal_range(req->second.sk);
  for (auto skiter = sk_er.first; skiter != sk_er.second; skiter++) {
    if (skiter->second == req) {
      dns_requests_by_sock_.erase(skiter);
      break;
    }
  }

  dns_requests_.erase(req);
}

/**
 * Removes all matching DNS Requests for a particular key
 *
 * @param[in] key Key of the DNS requests to remove
 */
void DnsRequests::remove_all_with_key(const dns_request_key &key)
{
  auto key_er = dns_requests_by_key_.equal_range(key);
  for (auto eriter = key_er.first; eriter != key_er.second;) {
    auto req = eriter->second;

    auto sk_er = dns_requests_by_sock_.equal_range(req->second.sk);
    for (auto skiter = sk_er.first; skiter != sk_er.second; skiter++) {
      if (skiter->second == req) {
        dns_requests_by_sock_.erase(skiter);
        break;
      }
    }

    dns_requests_.erase(req);

    eriter = dns_requests_by_key_.erase(eriter);
  }
}
