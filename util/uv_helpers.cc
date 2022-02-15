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

#include "util/uv_helpers.h"

#include <uv.h>

#include <condition_variable>
#include <mutex>
#include <tuple>

static void close_cb(uv_handle_t *const handle, void *const arg)
{
  if (!uv_is_closing(handle)) {
    uv_close(handle, nullptr);
  }
}

void close_uv_loop_cleanly(uv_loop_t *const loop)
{
  // TODO: Check `loop->stop_flag`?
  uv_walk(loop, &close_cb, nullptr);
  uv_run(loop, UV_RUN_DEFAULT);
  CHECK_UV(uv_loop_close(loop));
}

void sync_uv_run(::uv_loop_t &loop, std::function<void()> fn)
{
  std::mutex lock;
  std::condition_variable sync;

  struct Context {
    std::function<void()> fn;
    std::mutex lock;
    std::condition_variable sync;
    ::uv_async_t async;
  } context{
      .fn = std::move(fn),
  };

  CHECK_UV(::uv_async_init(&loop, &context.async, [](::uv_async_t *handle) {
    auto &context = *reinterpret_cast<Context *>(handle->data);
    {
      std::unique_lock<std::mutex> guard(context.lock);
      context.fn();
    }
    ::uv_close(reinterpret_cast<::uv_handle_t *>(&context.async), [](::uv_handle_t *handle) {
      auto &context = *reinterpret_cast<Context *>(handle->data);
      context.sync.notify_all();
    });
  }));
  context.async.data = &context;

  std::unique_lock<std::mutex> guard(context.lock);
  CHECK_UV(::uv_async_send(&context.async));
  context.sync.wait(guard);
}

struct libuv_error_category : std::error_category {
  char const *name() const noexcept override { return "libuv"; }

  std::string message(int condition) const override
  {
    std::ostringstream out;
    out << uv_error_t(condition);
    return out.str();
  }
};

static libuv_error_category libuv_category_;

std::error_category const &libuv_category() noexcept
{
  return libuv_category_;
}
