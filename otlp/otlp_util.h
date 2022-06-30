//
// Copyright 2022 Splunk Inc.
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

#include <google/protobuf/util/json_util.h>

template <typename T> std::string get_request_json(const T &request)
{
  std::string request_json_str;
  google::protobuf::util::JsonPrintOptions json_print_options;
  json_print_options.add_whitespace = true;
  json_print_options.always_print_primitive_fields = true;
  google::protobuf::util::MessageToJsonString(request, &request_json_str, json_print_options);
  return request_json_str;
}
