// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <generated/ebpf_net/matching/meta.h>

#include <util/json_converter.h>

int main()
{
  auto in = FileDescriptor::std_in();
  return json_converter::print_from_wire_format<ebpf_net::matching_metadata>(in, std::cout);
}
