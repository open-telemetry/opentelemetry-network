/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>
#include <string>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <google/protobuf/util/json_util.h>

template <typename T> 
std::optional<std::string> get_request_json(const T &request)
{
  std::string request_json_str;
  google::protobuf::util::JsonPrintOptions json_print_options;
  
  // Updated configuration options
  json_print_options.add_whitespace = true;
  
  // This option is not available in the protobuf library in Debian bullseye, 
  // we might bring it back as we update base distro
#if false
  json_print_options.always_print_fields_with_no_presence = true;
#endif

  json_print_options.preserve_proto_field_names = true;
  
  // Optional: control enum representation
  json_print_options.always_print_enums_as_ints = false;

  auto status = google::protobuf::util::MessageToJsonString(request, &request_json_str, json_print_options);
  if (!status.ok()) {
    std::cerr << "Failed to convert message to JSON: " << status.ToString() << std::endl;
    return "";
  }
  return request_json_str;
}
