// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <platform/types.h>

#include <util/enum.h>

// This file describes our outgoing metrics that are either hosted for
// prometheus to scrape, or sent over OTLP.
//
// They are broken up into enum "groups" (tcp, udp, dns, http, ebpf_net),
// and given a bit mask value.  The enum list ends with "all"
// with all bits turned on, representing every metric.
// A value of unknown is used mainly for string to enum conversions
// and denotes the conversion failed.
//
// These enums are used in "write_metrics.h" to send them out of the reducer
// to prometheus / OTLP.
// They are also used in "disable_metrics.h" to make the metric group
// participate in that code - that is to allow the metrics or group to be
// disabled.
//
// To add a new collection of metrics, follow how the other metric groups were
// added:
//
// #define ENUM_NAMESPACE reducer
// #define ENUM_NAME <GroupName>Metrics
// #define ENUM_TYPE std::uint32_t
// #define ENUM_ELEMENTS(X)
//   X(unknown, 0, "")                      \.
//   X(<enum-name1>, 0x001, <output-name1>) \.
//   X(<enum-name2>, 0x002, <output-name2>) \.
//   ...                                    \.
//   X(all, 0xFFFFFFFF, "<prefix>.all")
// #define ENUM_DEFAULT unknown
// #include <util/enum_operators.inl>
//
// If these metrics are to be disable-able, be sure to update disabled_metrics.h
// adding your group prefix there.

#define TCP_PREFIX "tcp."
#define ENUM_NAMESPACE reducer
#define ENUM_NAME TcpMetrics
#define ENUM_TYPE std::uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0, "")                                                                                                            \
  X(bytes, 0x001, TCP_PREFIX "bytes")                                                                                          \
  X(rtt_num_measurements, 0x002, TCP_PREFIX "rtt.num_measurements")                                                            \
  X(active, 0x004, TCP_PREFIX "active")                                                                                        \
  X(rtt_average, 0x008, TCP_PREFIX "rtt.average")                                                                              \
  X(packets, 0x010, TCP_PREFIX "packets")                                                                                      \
  X(retrans, 0x020, TCP_PREFIX "retrans")                                                                                      \
  X(syn_timeouts, 0x040, TCP_PREFIX "syn_timeouts")                                                                            \
  X(new_sockets, 0x080, TCP_PREFIX "new_sockets")                                                                              \
  X(resets, 0x100, TCP_PREFIX "resets")                                                                                        \
  X(all, 0xFFFFFFFF, TCP_PREFIX "all")
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef TCP_PREFIX

#define UDP_PREFIX "udp."
#define ENUM_NAMESPACE reducer
#define ENUM_NAME UdpMetrics
#define ENUM_TYPE std::uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0, "")                                                                                                            \
  X(bytes, 0x001, UDP_PREFIX "bytes")                                                                                          \
  X(packets, 0x002, UDP_PREFIX "packets")                                                                                      \
  X(active, 0x004, UDP_PREFIX "active")                                                                                        \
  X(drops, 0x008, UDP_PREFIX "drops")                                                                                          \
  X(all, 0xFFFFFFFF, UDP_PREFIX "all")
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef UDP_PREFIX

#define DNS_PREFIX "dns."
#define ENUM_NAMESPACE reducer
#define ENUM_NAME DnsMetrics
#define ENUM_TYPE std::uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0, "")                                                                                                            \
  X(client_duration_average, 0x001, DNS_PREFIX "client.duration.average")                                                      \
  X(server_duration_average, 0x002, DNS_PREFIX "server.duration.average")                                                      \
  X(active_sockets, 0x004, DNS_PREFIX "active_sockets")                                                                        \
  X(responses, 0x008, DNS_PREFIX "responses")                                                                                  \
  X(timeouts, 0x010, DNS_PREFIX "timeouts")                                                                                    \
  X(all, 0xFFFFFFFF, DNS_PREFIX "all")
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef DNS_PREFIX

#define HTTP_PREFIX "http."
#define ENUM_NAMESPACE reducer
#define ENUM_NAME HttpMetrics
#define ENUM_TYPE std::uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0, "")                                                                                                            \
  X(client_duration_average, 0x001, HTTP_PREFIX "client.duration.average")                                                     \
  X(server_duration_average, 0x002, HTTP_PREFIX "server.duration.average")                                                     \
  X(active_sockets, 0x004, HTTP_PREFIX "active_sockets")                                                                       \
  X(status_code, 0x008, HTTP_PREFIX "status_code")                                                                             \
  X(all, 0xFFFFFFFF, HTTP_PREFIX "all")
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef HTTP_PREFIX
