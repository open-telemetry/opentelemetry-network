// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/test_channel.h>
#include <generated/ebpf_net/ingest/meta.h>
#include <jitbuf/jb.h>

#include <util/json_converter.h>

namespace channel {

TestChannel::TestChannel(std::optional<std::reference_wrapper<uv_loop_t>> loop, IntakeEncoder encoder)
    : loop_(loop), encoder_(encoder)
{}

std::error_code TestChannel::send(const u8 *data, int size)
{
  try {
    ++num_sends_;

    LOG::trace("TestChannel::send() num_sends_ {}", num_sends_);

    switch (encoder_) {
    case IntakeEncoder::binary: {
      auto rpc = reinterpret_cast<jb_rpc const *>(data + sizeof(u64)); // + sizeof(u64) to skip past timestamp
      LOG::trace("TestChannel::send() rpc: rpc_id {} size {}", rpc->rpc_id, rpc->size);

      binary_messages_.emplace_back(data, data + size);

      std::stringstream ss;
      json_converter::WireToJsonConverter<ebpf_net::ingest_metadata> converter(ss);

      if (auto const handled = converter.process(reinterpret_cast<char const *>(data), size); !handled) {
        if (handled.error().value() == EAGAIN) {
          LOG::error("TestChannel::send() converter.process() returned EAGAIN");
        } else {
          LOG::error("TestChannel::send() error while handling message: {}", handled.error());
        }
        ++num_failed_sends_;
      } else {
        LOG::trace("TestChannel::send() binary format msg converted to JSON {}", log_waive(ss.str()));

        std::string str = "[" + ss.str() + "]";
        nlohmann::json const objects = nlohmann::json::parse(str);
        for (auto const &object : objects) {
          ++message_counts_[object["name"]];
          json_messages_.push_back(object);
          if (sent_msg_cb_) {
            sent_msg_cb_(object);
          }
        }
      }
    } break;
    default:
      ++num_failed_sends_;
      LOG::error("unknown IntakeEncoder {}", encoder_);
      break;
    }
  } catch (std::exception &ex) {
    ++num_failed_sends_;
    LOG::error("exception caught in TestChannel::send() {}", ex.what());
  }

  return {};
}

void TestChannel::close() {}

std::error_code TestChannel::flush()
{
  return {};
}

u64 TestChannel::get_num_sends()
{
  return num_sends_;
};

u64 TestChannel::get_num_failed_sends()
{
  return num_failed_sends_;
}

std::stringstream &TestChannel::get_ss()
{
  return ss_;
}

TestChannel::MessageCountsType &TestChannel::get_message_counts()
{
  return message_counts_;
}

void TestChannel::binary_messages_for_each(std::function<void(BinaryMessageType const &)> const &cb)
{
  for (auto const &msg : binary_messages_) {
    cb(msg);
  }
}

TestChannel::JsonMessagesType &TestChannel::get_json_messages()
{
  return json_messages_;
}

void TestChannel::json_messages_for_each(std::function<void(JsonMessageType const &)> const &cb)
{
  for (auto const &msg : json_messages_) {
    cb(msg);
  }
}

void TestChannel::set_sent_msg_cb(std::function<void(nlohmann::json const &)> const &sent_msg_cb)
{
  sent_msg_cb_ = sent_msg_cb;
}

void TestChannel::connect(Callbacks &callbacks)
{
  auto fake_connected_cb = [&callbacks]() {
    LOG::trace("TestChannel::connect() fake_connected_cb() - calling callbacks.on_connect()");
    callbacks.on_connect();
  };

  fake_connected_cb_timer_ = std::make_unique<scheduling::Timer>(*loop_, fake_connected_cb);
  auto delay_sec = 1;
  if (auto const result = fake_connected_cb_timer_->defer(std::chrono::seconds(delay_sec))) {
    LOG::trace("successfully scheduled fake_connected_cb() {} second(s) from now", delay_sec);
  } else {
    throw std::runtime_error("failed to schedule fake_connected_cb()");
  }
}

in_addr_t const *TestChannel::connected_address() const
{
  return 0;
}

} // namespace channel
