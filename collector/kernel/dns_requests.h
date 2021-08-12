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

#pragma once

#include <list>
#include <unordered_map>

#include <platform/platform.h>

class DnsRequests {
  // Type declarations
public:
  struct dns_request_key {
    u16 qid;          // transaction id
    u16 type;         // query type
    std::string name; // query data
    bool is_rx;       // was this request 'sent (client)' or 'received (server)'
  };

  struct dns_request_value {
    u64 timestamp_ns; // when the query was made according to bpf
    u64 sk;           // socket that sent the dns request
  };

protected:
  struct dns_request_key_hash {
    size_t operator()(const dns_request_key &k) const noexcept;
  };
  struct dns_request_key_equal_to {
    bool operator()(const dns_request_key &k, const dns_request_key &k2) const noexcept;
  };

  typedef std::list<std::pair<dns_request_key, dns_request_value>> DnsRequestsList;
  typedef std::unordered_multimap<dns_request_key, DnsRequestsList::iterator, dns_request_key_hash, dns_request_key_equal_to>
      DnsRequestsByKeyMap;
  typedef std::unordered_multimap<u64, DnsRequestsList::iterator> DnsRequestsBySockMap;

public:
  typedef DnsRequestsList::iterator Request;

  // Public interface
public:
  void add(const dns_request_key &key, const dns_request_value &value);
  void lookup(const dns_request_key &key, std::list<Request> &out);
  void lookup_older_than(u64 timestamp_ns, std::list<Request> &out);
  void lookup_socket(u64 sk, std::list<Request> &out);
  void remove(const Request &req);
  void remove_all_with_key(const dns_request_key &key);

  // Protected member variables
protected:
  DnsRequestsByKeyMap dns_requests_by_key_;
  DnsRequestsList dns_requests_;
  DnsRequestsBySockMap dns_requests_by_sock_;
};
