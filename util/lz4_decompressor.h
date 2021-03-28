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

#pragma once

#include <platform/platform.h>

#include <lz4frame.h>

#include <cstdint>
#include <tuple>

class Lz4Decompressor {
public:
  explicit Lz4Decompressor(size_t buf_capacity);
  ~Lz4Decompressor();

  const u8 *output_buf() const;
  size_t output_buf_size() const;

  // Decompresses |data| of size |data_len|.
  // The # of bytes that have been decompressed is stored in |consumed_len|.
  //
  // Returns LZ4 error, if an error happened, 0 otherwise.
  size_t process(const u8 *data, size_t data_len, size_t *consumed_len);

  // Discards |len| bytes of data in output_buf.
  void discard(size_t len);

private:
  const size_t output_buf_capacity_;
  size_t tail_loc_;

  LZ4F_dctx *ctx_;
  u8 *output_buf_;
};
