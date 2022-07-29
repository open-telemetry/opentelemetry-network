// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <platform/platform.h>

#include "spdlog/common.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <collector/kernel/bpf_src/render_bpf.h>
#include <collector/kernel/perf_reader.h>
#include <collector/kernel/tcp_data_handler.h>
#include <iostream>
#include <stdexcept>
#include <util/ip_address.h>
#include <util/log.h>
#include <util/lookup3.h>
#include <uv.h>

#include "protocols/protocol_handler_base.h"
#include "protocols/protocol_handler_http.h"
#include "protocols/protocol_handler_unknown.h"

bool TCPDataHandler::tcp_control_key_t_comparator::operator()(const tcp_control_key_t &a, const tcp_control_key_t &b) const
{
  return a.sk < b.sk;
}

TCPDataHandler::TCPDataHandler(
    uv_loop_t &loop,
    ebpf::BPFModule &bpf_module,
    ::flowmill::ingest::Writer &writer,
    PerfContainer &container,
    logging::Logger &log)
    : loop_(loop), bpf_module_(bpf_module), writer_(writer), container_(container), log_(log)
{
  // Get tcp control hash table
  ebpf::TableStorage::iterator it;
  ebpf::Path path({bpf_module.id(), "_tcp_control"});
  if (!bpf_module.table_storage().Find(path, it)) {
    throw std::runtime_error("missing _tcp_control hash table");
  }
  tcp_control_desc_ = it->second.dup();
}

TCPDataHandler::~TCPDataHandler() {}

std::shared_ptr<ProtocolHandlerBase>
TCPDataHandler::create_protocol_handler(int protocol, const tcp_control_key_t &key, u32 pid)
{
  switch (protocol) {
  case TCPPROTO_HTTP:
    return std::make_shared<ProtocolHandler_HTTP>(this, key, pid);
  case TCPPROTO_UNKNOWN:
    return std::make_shared<ProtocolHandler_UNKNOWN>(this, key, pid);
  default:
    throw std::runtime_error("invalid protocol");
  }
  return nullptr;
}

void TCPDataHandler::upgrade_protocol_handler(
    std::shared_ptr<ProtocolHandlerBase> new_handler, std::shared_ptr<ProtocolHandlerBase> original_handler)
{
  tcp_control_key_t key = original_handler->control_key();
#ifndef NDEBUG
  tcp_control_key_t newkey = new_handler->control_key();
  assert(key.sk == newkey.sk);
#endif

  auto it = protocol_handlers_.find(key);
  if (it == protocol_handlers_.end()) {
    throw std::runtime_error("update from invalid protocol");
  }

  it->second = new_handler;
}

// toggle enabling one side of a stream
void TCPDataHandler::enable_stream(const tcp_control_key_t &key, STREAM_TYPE stream_type, bool enable)
{
  // Get the file descriptor for the tcp control map
  int fd = (int)tcp_control_desc_.fd;

  // Get the key pointer in a form that bpf can accept
  void *keyptr = const_cast<void *>(static_cast<const void *>(&key));

  // Get the previous value
  tcp_control_value_t value;
  int err;
  if ((err = bpf_lookup_elem(fd, keyptr, &value)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, lookup failed, sk={:x}, err={}", key.sk, err);
    return;
  }

  // Change the desired values
  value.streams[stream_type].enable = enable;

  // Update the value if it is still in the table
  if ((err = bpf_update_elem(fd, keyptr, &value, BPF_EXIST)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, update failed, sk={:x}, err={}", key.sk, err);
    return;
  }

  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "enable_stream: sk={}, stream_type={}, enable={}",
      key.sk,
      stream_type_to_string(stream_type),
      enable ? "true" : "false");
}

// toggle enabling both sides of a stream
void TCPDataHandler::enable_stream(const tcp_control_key_t &key, bool enable)
{
  // Get the file descriptor for the tcp control map
  int fd = (int)tcp_control_desc_.fd;

  // Get the key pointer in a form that bpf can accept
  void *keyptr = const_cast<void *>(static_cast<const void *>(&key));

  // Get the previous value
  tcp_control_value_t value;
  int err;
  if ((err = bpf_lookup_elem(fd, keyptr, &value)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, lookup failed, sk={:x}, err={}", key.sk, err);
    return;
  }

  // Change the desired values
  value.streams[ST_SEND].enable = enable;
  value.streams[ST_RECV].enable = enable;

  // Update the value if it is still in the table
  if ((err = bpf_update_elem(fd, keyptr, &value, BPF_EXIST)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, update failed, sk={:x}, err={}", key.sk, err);
    return;
  }
  LOG::debug_in(
      AgentLogKind::PROTOCOL, "enable_stream: sk={:x}, stream_type=BOTH, enable={}", key.sk, enable ? "true" : "false");
}

void TCPDataHandler::update_stream_start(const tcp_control_key_t &key, STREAM_TYPE stream_type, u64 start)
{
  // Get the file descriptor for the tcp control map
  int fd = (int)tcp_control_desc_.fd;

  // Get the key pointer in a form that bpf can accept
  void *keyptr = const_cast<void *>(static_cast<const void *>(&key));

  // Get the previous value
  tcp_control_value_t value;
  int err;
  if ((err = bpf_lookup_elem(fd, keyptr, &value)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, lookup failed, sk={:x}, err={}", key.sk, err);
    return;
  }

  // Change the desired values
  value.streams[stream_type].start = start;

  // Update the value if it is still in the table
  if ((err = bpf_update_elem(fd, keyptr, &value, BPF_EXIST)) < 0) {
    // It's okay for this to fail, it means that BPF deleted the tcp connection record
    // before this got called, which means we should do nothing in this case
    LOG::debug_in(
        AgentLogKind::PROTOCOL, "enable_stream called on non-existing key, update failed, sk={:x}, err={}", key.sk, err);
    return;
  }

  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "update_stream_start: sk={:x}, stream_type={}, start={}",
      key.sk,
      stream_type_to_string(stream_type),
      start);
}

void TCPDataHandler::process(
    size_t idx, u64 tstamp, u64 sk, u32 pid, u32 length, u64 offset, STREAM_TYPE stream_type, CLIENT_SERVER_TYPE client_server)
{
  // Send to the appropriate protocol handler
  tcp_control_key_t key{.sk = sk};

  // find the appropriate protocol handler
  auto phiter = protocol_handlers_.find(key);
  if (phiter == protocol_handlers_.end()) {
    // data for new socket, so create a protocol handler for it
    auto ret = protocol_handlers_.insert(std::make_pair(key, create_protocol_handler(TCPPROTO_UNKNOWN, key, pid)));
    phiter = ret.first;
  }

  // Get the data ring for the same cpu as the control ring
  // we're on to ensure we get the right data
  PerfRing &ring = container_.data_ring(idx);

  u32 length_remaining = length;
  u64 current_offset = offset;

  while (length_remaining > 0) {
    auto ring_size = ring.peek_size();

    if (ring_size == -ENOENT) {
      LOG::debug_in(AgentLogKind::PROTOCOL, "TCPDataHandler::process: ring buffer is empty");
      break;
    }

    // If we have an sample, process it
    // peek_type assumes ring is non-empty
    int type = ring.peek_type();

    if (type == PERF_RECORD_SAMPLE) {

      u32 padded_chunk_length = ring_size;

      // PERF_SAMPLE_RAW adds 32 bits of length per documentation of perf_event_open
      const unsigned int min_length = sizeof(u32);
      // round up max_length to a multiple of 8 bytes for padding
      const unsigned int max_length = ((sizeof(u32) + DATA_CHANNEL_CHUNK_MAX) + 7) & ~7;

      // Ensure we don't overflow
      if (padded_chunk_length < min_length) {
        log_.error("got message < sizeof header: padded_chunk_length({}) < min_length({})", padded_chunk_length, min_length);
        return;
      }
      if (padded_chunk_length > max_length) {
        log_.error("got message > sizeof message: padded_chunk_length({}) > max_length({})", padded_chunk_length, max_length);
        return;
      }

      // process tcp data
      LOG::debug_in(AgentLogKind::PROTOCOL, "tcp_data_handler: processing data (padded_chunk_length={})", padded_chunk_length);

      char buf[max_length];

      /* copy into buffer */
      ring.peek_copy(buf, 0, padded_chunk_length);
      /* release the element */
      ring.pop();

      // get pointer to data and length of data
      data_channel_header_t *header = (data_channel_header_t *)(buf + sizeof(u32));
      const u8 *data = (const u8 *)(buf + sizeof(u32) + sizeof(data_channel_header_t));
      u32 data_len = header->length;

      // make sure we don't read past end
      const unsigned int chunk_length = data_len + sizeof(u32) + sizeof(data_channel_header_t);
      if (chunk_length > padded_chunk_length) {
        log_.error(
            "got chunk_length > padded_chunk_length: chunk_length({}) > padded_chunk_length({})",
            chunk_length,
            padded_chunk_length);
        return;
      }

      // make sure our read won't take us past the length remaining on this data
      if (data_len > length_remaining) {
        // there is more data in the buffer than we need
        // just process the remaining length
        data_len = length_remaining;
      }

      // process the data with the protocol handler
      // possibly upgrading the handler
      ProtocolHandlerBase::ptr_type phb = phiter->second;

      bool do_upgrade;
      do {
        do_upgrade = false;

        if (client_server == SC_SERVER) {
          phb->handle_server_data(tstamp, current_offset, stream_type, data, data_len);
        } else {
          phb->handle_client_data(tstamp, current_offset, stream_type, data, data_len);
        }

        ProtocolHandlerBase::ptr_type upgrade = phb->get_upgrade();
        if (upgrade) {
          upgrade_protocol_handler(upgrade, phb);
          phb = upgrade;
          do_upgrade = true;
        }
      } while (do_upgrade);

      // offset for the next chunk
      current_offset += data_len;
      length_remaining -= data_len;

    } else if (type == PERF_RECORD_LOST) {

      u64 n_lost = ring.peek_aligned_u64(sizeof(u64));
      ring.pop();

      lost_record_total_count_ = lost_record_total_count_ + n_lost;

      log_.warn("tcp_data_handler: lost {} records ({} total)", n_lost, lost_record_total_count_);
    } else {
      log_.error("tcp_data_handler: unexpected record type {}", type);
      // skip this one
      ring.pop();
    }
  }
}

void TCPDataHandler::handle_close_socket(u64 sk)
{
  // Send to the appropriate protocol handler
  tcp_control_key_t key{
      .sk = sk,
  };

  // end-of-socket
  auto phiter = protocol_handlers_.find(key);
  if (phiter == protocol_handlers_.end()) {
    // This is okay since sockets that send no data won't have a protocol handler
    // LOG::debug_in(AgentLogKind::PROTOCOL, "missing protocol handler for key(sk={:x})", key.sk);
  } else {
    // remove protocol handler at end of socket
    protocol_handlers_.erase(phiter);
  }
}
