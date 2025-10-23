/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

std::ostream &operator<<(std::ostream &out, google::protobuf::Message const &message)
{
  google::protobuf::io::OstreamOutputStream output(&out);

  google::protobuf::TextFormat::Printer printer;
  printer.SetExpandAny(true);

  printer.Print(message, &output);

  return out;
}

template <typename T>
struct fmt::formatter<T, char, std::enable_if_t<std::is_base_of_v<google::protobuf::Message, T>>> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(const T &message, FormatContext &ctx) const
  {
    std::ostringstream oss;
    oss << message;
    return fmt::formatter<std::string>::format(oss.str(), ctx);
  }
};
