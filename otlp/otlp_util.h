/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>
#include <string>
#include <google/protobuf/util/json_util.h>

template <typename T> 
std::optional<std::string> get_request_json(const T &request)
{
  std::string request_json_str;
  google::protobuf::util::JsonPrintOptions json_print_options;
  
  // Updated configuration options
  json_print_options.add_whitespace = true;
  json_print_options.preserve_proto_field_names = true;
  
  // Instead of always_print_primitive_fields, use print_default_values
  json_print_options.print_default_values = true;

  // Optional: control enum representation
  json_print_options.always_print_enums_as_ints = false;

  // Status handling is explicit
  google::protobuf::util::Status status = 
    google::protobuf::util::MessageToJsonString(
      request, 
      &request_json_str, 
      json_print_options
    );

  // Check status and return optional
  if (status.ok()) {
    return request_json_str;
  } else {
    // Log the error or handle it as needed
    std::cerr << "JSON conversion error: " << status.ToString() << std::endl;
    return std::nullopt;
  }
}
