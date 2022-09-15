/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/buffer.h>
#include <util/expected.h>
#include <util/file_ops.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/meta.h>
#include <util/raw_json.h>

#include <iomanip>
#include <iostream>
#include <string_view>
#include <system_error>

namespace json_converter {

template <typename App> class WireToJsonConverter {
  template <typename Message> static void convert_message(void *ctx, std::uint64_t timestamp, char *data)
  {
    auto context = reinterpret_cast<WireToJsonConverter *>(ctx);

    if (context->first_) {
      context->first_ = false;
    } else {
      context->out_ << ',';
    }

    context->out_ << "{\"name\":\"" << Message::name << "\""
                  << ",\"rpc_id\":" << Message::rpc_id << ",\"timestamp\":" << timestamp;

    auto const message = reinterpret_cast<typename Message::parsed_message const *>(data);

    if constexpr (Message::has_reference) {
      print_json_value<typename Message::reference::type>(context->out_ << ",\"ref\":", Message::reference::get(message));
    }

    context->out_ << ",\"data\":{";

    meta::foreach<typename Message::fields>([&](auto tag) {
      using field = decltype(meta::tag_type(tag));

      if constexpr (field::index) {
        context->out_ << ',';
      }
      context->out_ << '"' << field::name << "\":";

      print_json_value<typename field::type>(context->out_, field::get(message));
    });
    context->out_ << "}}";
  }

public:
  template <typename... Args>
  WireToJsonConverter(std::ostream &out, Args &&...args)
      : out_(out), transform_builder_(std::forward<Args>(args)...), protocol_(transform_builder_)
  {
    meta::foreach<typename App::messages>([this](auto tag) {
      using message = decltype(meta::tag_type(tag));
      protocol_.add_handler(message::rpc_id, this, &convert_message<message>);
    });

    protocol_.insert_no_auth_identity_transforms();
    protocol_.insert_need_auth_identity_transforms();
  }

  Expected<std::size_t, std::error_code> process(char const *data, std::size_t size)
  {
    auto const result = protocol_.handle_multiple(data, size).result;
    if (result < 0) {
      return {unexpected, -result, std::generic_category()};
    }
    return static_cast<std::size_t>(result);
  }

private:
  std::ostream &out_;
  bool first_ = true;
  typename App::transform_builder transform_builder_;
  typename App::protocol protocol_;
};

/**
 * Converts all wire messages from `in` for the render application metadata
 * `App` to JSON format and prints output to `out`.
 *
 * Example:
 *
 *  auto in = FileDescriptor::std_in();
 *  llvm::LLVMContext llvm;
 *  json_converter::print_from_wire_format<ebpf_net::flowtune_metadata>(in, std::cout, llvm);
 */
template <typename App, std::size_t BufferSize = 4096, typename... Args>
int print_from_wire_format(FileDescriptor &in, std::ostream &out, Args &&...args)
{
  json_converter::WireToJsonConverter<App> converter(out, std::forward<Args>(args)...);
  buffer::Buffer<BufferSize> buffer;

  out << '[';
  for (bool eof = false; !eof;) {
    auto writable = buffer.compact().writable();

    if (auto const read = in.read_all(writable.data(), writable.size()).try_raise().value()) {
      buffer.commit(read);
    } else {
      eof = true;
    }

    auto const available = buffer.available();
    if (auto const handled = converter.process(available.data(), available.size()); !handled) {
      if (handled.error().value() == EAGAIN) {
        if (buffer.consumed().empty()) {
          LOG::error("buffer too small with {} bytes", buffer.size());
          return ENOBUFS;
        }

        continue;
      }
      LOG::error("error while handling message: {}", handled.error());
      return handled.error().value();
    } else if (*handled > 0) {
      buffer.consume(*handled);
    }
  }

  out << ']';
  out.flush();
  return 0;
}

} // namespace json_converter
