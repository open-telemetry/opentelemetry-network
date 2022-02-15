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

#include <channel/test_channel.h>
#include <generated/flowmill/ingest/meta.h>
#include <jitbuf/jb.h>
#include <tools/json_converter.h>

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

      char msg[size];
      memcpy(msg, data, size);

      std::stringstream ss;
      llvm::LLVMContext llvm;
      json_converter::WireToJsonConverter<flowmill::ingest_metadata> converter(ss, llvm);

      if (auto const handled = converter.process(msg, size); !handled) {
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
        }
      }
    } break;
    case IntakeEncoder::otlp_log: {
      auto msg_size = static_cast<std::string_view::size_type>(size);
      std::string_view const msg{reinterpret_cast<char const *>(data), msg_size};

      ss_ << msg;
      LOG::trace("TestChannel::send() otlp_log format msg {}", msg);

      auto pos = msg.find_first_of("{");
      if (pos == std::string::npos) {
        ++num_failed_sends_;
        LOG::error("cannot parse msg {}", msg);
        return {};
      }
      std::string_view json_msg(&msg[pos], msg_size - pos);
      nlohmann::json const object = nlohmann::json::parse(json_msg);

      /*
        {
        "resourceLogs": [
        {
        "instrumentationLibraryLogs": [
        {
        "logs": [
        {
        "name": "nic_stats",
      */
      for (auto const &rl : object["resourceLogs"]) {
        for (auto const &ill : rl["instrumentationLibraryLogs"]) {
          for (auto const &log : ill["logs"]) {
            ++message_counts_[log["name"]];
            json_messages_.push_back(log);
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

TestChannel::JsonMessagesType &TestChannel::get_json_messages()
{
  return json_messages_;
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
