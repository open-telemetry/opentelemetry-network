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

#include "id_mapping.h"

#include "code_utils.h"
#include <stdexcept>
#include <string>

namespace jitbuf {

IdMapping::IdMapping(const google::protobuf::FileDescriptor *file)
{
  const google::protobuf::Descriptor *namespace_msg = NULL;

  /* first, search dependencies for the special message "jitbuf_namespace" */
  for (int i = 0; i < file->dependency_count(); i++) {
    auto dep_file = file->dependency(i);
    namespace_msg = dep_file->FindMessageTypeByName("jitbuf_namespace");
    if (namespace_msg != NULL)
      break;
  }

  /* did we find it? */
  if (namespace_msg == NULL)
    throw std::runtime_error("could not find jitbuf_namespace message");

  /* get the package name for this file */
  std::vector<std::string> packages = CodeUtils::split_package(file->package());

  /* we require a non-empty package name */
  if (packages.size() == 0)
    throw std::runtime_error("expected non-empty package name");

  /* traverse the containing packages until reaching the innermost package */
  for (int i = 0; i < packages.size() - 1; i++) {
    namespace_msg = namespace_msg->FindNestedTypeByName(packages[i]);
    if (namespace_msg == NULL) {
      std::ostringstream oss;
      oss << "cannot find the " << i << "-depth package nesting: " << packages[i];
      throw std::runtime_error(oss.str());
    }
  }

  /* now find the field corresponding to the innermost package */
  auto field = namespace_msg->FindFieldByName(packages.back());
  if (field == NULL)
    throw std::runtime_error("could not find innermost package: " + packages.back());

  /* the ID ranges are in the default option */
  if (field->has_default_value() == false)
    throw std::runtime_error("package does not have a default value specifying ranges");

  auto ranges_str = field->default_value_string();

  auto ranges = CodeUtils::split(ranges_str, ',');
  for (auto range : ranges) {
    auto ends = CodeUtils::split(range, '-');
    if (ends.size() > 2)
      throw std::runtime_error("got a range with more than one hyphen");

    /* sanity check the string */
    for (auto one_end : ends) {
      if (one_end.size() == 0)
        throw std::runtime_error("one side of the range was unspecified");
      for (auto ch : one_end)
        if (!isdigit(ch))
          throw std::runtime_error("range contained non-numerals");
    }

    /* single number is equivalent to first=last */
    if (ends.size() == 1)
      ends.push_back(ends.front());

    int a = std::stoi(ends[0]);
    int b = std::stoi(ends[1]);
    if (b < a)
      throw std::runtime_error("interval a-b, there should hold b >= a");
    first.push_back(a);
    last.push_back(b);
  }
}

u32 IdMapping::map(u32 n)
{
  /* find the containing range */
  u32 prev_intervals = 0;
  for (int i = 0; i < first.size(); i++) {
    u32 a = first[i];
    u32 b = last[i];

    if (a + (n - prev_intervals) <= b)
      return a + (n - prev_intervals);

    prev_intervals += b - a + 1;
  }

  /* ranges are not big enough for n */
  throw std::runtime_error("rpc id too large for specified namespace");
}

} // namespace jitbuf
