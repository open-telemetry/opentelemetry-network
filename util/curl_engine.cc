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

#include <util/curl_engine.h>
#include <util/utility.h>

#include <stdexcept>
#include <unordered_set>

#include <util/log.h>

namespace {

static constexpr size_t kWriteBufferCapacity = CURL_MAX_WRITE_SIZE;

// The global atomic flag to make sure curl_global_init() is called once
// and only once.
std::atomic_flag has_curl_global_init = ATOMIC_FLAG_INIT;

// Forward define.
static void on_uv_poll(uv_poll_t *poll_handle, int status, int events);

// Implements CurlEngine interface.
class CurlEngineImpl : public CurlEngine {
public:
  explicit CurlEngineImpl(uv_loop_t *loop);
  ~CurlEngineImpl() override;

  CurlEngineStatus schedule_fetch(FetchRequest &request) override;
  CurlEngineStatus cancel_fetch(FetchRequest &request) override;

  // Functions used by libuv & libcurl callbacks.
  void start_timer(u64 timeout_ms);
  void stop_timer();
  void on_timeout();

  void start_poll(curl_socket_t socket_fd, int events);
  void remove_poll(uv_poll_t *poll_handle);
  void on_poll(uv_poll_t *poll_handle, int flags);

private:
  void handle_curl_info_queue();

  size_t num_active_fetches_ = 0;
  uv_loop_t *loop_;
  CURLM *curl_handle_;
  uv_timer_t timer_;
};
} // namespace

CurlEngine::FetchRequest::FetchRequest(std::string target, DataAvailableFn available_fn, FetchDoneFn done_fn)
    : easy_handle_(curl_easy_init()),
      target_(std::move(target)),
      available_fn_(std::move(available_fn)),
      done_fn_(std::move(done_fn))
{}

CurlEngine::FetchRequest::~FetchRequest()
{
  // curl cleanup functions are no-op when the handle is null
  if (easy_handle_) {
    curl_easy_cleanup(easy_handle_);
  }
  if (headers_) {
    curl_slist_free_all(headers_);
  }
}

bool CurlEngine::FetchRequest::add_header(char const *header)
{
  if (auto head = curl_slist_append(headers_, header)) {
    headers_ = head;
    return true;
  } else {
    LOG::error("Cannot insert HTTP header to libcurl request.");
    return false;
  }
}

void CurlEngine::FetchRequest::http_proxy(std::string host, std::uint16_t port)
{
  proxy_ = std::move(host);
  proxy_port_ = port;
  proxy_type_ = CURLPROXY_HTTP;
}

// curl_write() is a callback passed to libcurl, which is invoked when
// server returns data.
size_t CurlEngine::FetchRequest::curl_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  LOG::trace_in(Utility::curl, "{}(): {}", __func__, size * nmemb);

  auto *request = (CurlEngine::FetchRequest *)userdata;

  // See https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
  // |size| is actually always 1, but handle it anyway.
  size_t incoming_size = size * nmemb;
  request->available_fn_(ptr, incoming_size);

  return incoming_size;
}

// |unix_socket|: if not empty, the unix socket the engine should talk to.
void CurlEngine::FetchRequest::unix_socket(std::string_view path)
{
  unix_socket_ = std::string(path);
}

void CurlEngine::FetchRequest::debug_mode(bool debug)
{
  curl_easy_setopt(easy_handle_, CURLOPT_VERBOSE, static_cast<long>(debug));
}

void CurlEngine::FetchRequest::done(CurlEngineStatus status, bool success, std::string_view error)
{
  long response_code = -1;
  if (success && !get_info(CURLINFO_RESPONSE_CODE, response_code)) {
    error = "fetch successful, but unable to fetch response code";
  }

  done_fn_(status, response_code, error);
}

CURL *CurlEngine::FetchRequest::prepare()
{
  // Note: unfortunately libcurl does not validate the URL.
  // This function call always returns OK.
  // See: https://curl.haxx.se/libcurl/c/CURLOPT_URL.html
  set_option(CURLOPT_URL, target_.c_str());
  set_option(CURLOPT_PRIVATE, this);
  if (headers_) {
    set_option(CURLOPT_HTTPHEADER, headers_);
  }

  // Curstomized write callback.
  // https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
  set_option(CURLOPT_WRITEFUNCTION, curl_write);
  set_option(CURLOPT_WRITEDATA, this);

  // Uses Unix domain socket for communication
  // https://curl.haxx.se/libcurl/c/CURLOPT_UNIX_SOCKET_PATH.html
  if (!unix_socket_.empty()) {
    set_option(CURLOPT_UNIX_SOCKET_PATH, unix_socket_.c_str());
  }

  if (!proxy_.empty()) {
    set_option(CURLOPT_PROXY, proxy_.c_str());
    set_option(CURLOPT_PROXYPORT, static_cast<long>(proxy_port_));
    set_option(CURLOPT_PROXYTYPE, proxy_type_);
  }

  // Holds any error message from libcurl.
  set_option(CURLOPT_ERRORBUFFER, error_message_);

  return easy_handle_;
}

std::unique_ptr<CurlEngine> CurlEngine::create(uv_loop_t *loop)
{
  std::unique_ptr<CurlEngine> engine(new CurlEngineImpl(loop));
  return engine;
}

namespace {
// curl_start_timer() is a callback passed to libcurl. Libcurl uses it to
// inform libuv to start or stop timer.
static int curl_start_timer(CURLM *multi, long timeout_ms, void *userp)
{
  LOG::trace_in(Utility::curl, "curl_start_timer(): {}ms", timeout_ms);

  CurlEngineImpl *engine = (CurlEngineImpl *)userp;
  if (timeout_ms < 0) {
    engine->stop_timer();
  } else {
    // timeout_ms ==0 means we can invoke the timer callback right away.
    // But we will run it via the uv_loop moment later.
    engine->start_timer(timeout_ms);
  }
  return 0;
}

// curl_handle_socket() is a callback passed to libcurl. Libcurl uses it to
// signal that a socket_fd has become available, or is distroyed, libuv should
// start, or remove, a poll routine.
static int curl_handle_socket(CURL *easy, curl_socket_t socket_fd, int action, void *userp, void *socketp)
{
  LOG::trace_in(Utility::curl, "curl_handle_socket(), action: {}", action);

  CurlEngineImpl *engine = (CurlEngineImpl *)userp;
  uv_poll_t *poll_handle = (uv_poll_t *)(socketp);

  int events = 0;

  switch (action) {
  case CURL_POLL_IN:
  case CURL_POLL_INOUT:
  case CURL_POLL_OUT:
    if (action != CURL_POLL_IN) {
      events |= UV_WRITABLE;
    }

    if (action != CURL_POLL_OUT) {
      events |= UV_READABLE;
    }
    if (poll_handle == nullptr) {
      // Start a fresh poll.
      engine->start_poll(socket_fd, events);
    } else {
      // Call uv_poll_start() again to update the event masks.
      int res = uv_poll_start(poll_handle, events, on_uv_poll);
      if (res != 0) {
        LOG::error("Cannot start poll. {}", uv_err_name(res));
        return 0;
      }
    }
    break;
  case CURL_POLL_REMOVE:
    assert(poll_handle != nullptr);
    engine->remove_poll(poll_handle);
    break;
  case CURL_POLL_NONE:
    break;
  default:
    LOG::warn("Unexpected socket action {}", action);
  }

  return 0;
}

// on_uv_timeout() is a callback from libuv to signal the timer has fired.
static void on_uv_timeout(uv_timer_t *timer)
{
  LOG::trace_in(Utility::curl, "on_uv_timeout()");
  auto *engine = (CurlEngineImpl *)timer->data;
  engine->on_timeout();
}

// on_uv_poll() is a callback from libuv to signal the socket_fd status changes.
static void on_uv_poll(uv_poll_t *poll_handle, int status, int events)
{
  int flags = 0;

  if (events & UV_READABLE) {
    flags |= CURL_CSELECT_IN;
  }
  if (events & UV_WRITABLE) {
    flags |= CURL_CSELECT_OUT;
  }

  auto *engine = (CurlEngineImpl *)poll_handle->data;
  engine->on_poll(poll_handle, flags);
}

CurlEngineImpl::CurlEngineImpl(uv_loop_t *loop) : loop_(loop)
{
  if (loop == nullptr) {
    LOG::error("libuv loop is not specified.");
    throw std::runtime_error("libuv loop is not specified.");
    return;
  }

  if (!has_curl_global_init.test_and_set()) {
    if (curl_global_init(CURL_GLOBAL_ALL)) {
      LOG::error("Could not init curl");
      throw std::runtime_error("Could not init curl.");
      return;
    }

    if (atexit(curl_global_cleanup) != 0) {
      LOG::error("Could not setup curl_global_cleanup.");
      throw std::runtime_error("Cloud not setup curl_global_cleanup.");
      return;
    }
  }

  int res = uv_timer_init(loop_, &timer_);
  if (res != 0) {
    LOG::error("CurlEngineImpl: Cannot set up timer: {}", uv_err_name(res));
    throw std::runtime_error("Cannot set up timer.");
    return;
  }
  timer_.data = this;

  curl_handle_ = curl_multi_init();
  curl_multi_setopt(curl_handle_, CURLMOPT_SOCKETFUNCTION, curl_handle_socket);
  curl_multi_setopt(curl_handle_, CURLMOPT_SOCKETDATA, (void *)this);

  curl_multi_setopt(curl_handle_, CURLMOPT_TIMERFUNCTION, curl_start_timer);
  curl_multi_setopt(curl_handle_, CURLMOPT_TIMERDATA, (void *)this);
}

CurlEngineImpl::~CurlEngineImpl()
{
  if (curl_handle_ == nullptr) {
    return;
  }

  if (num_active_fetches_ != 0) {
    LOG::error("{} on-going fetches when the CurlEngine is destroyed", num_active_fetches_);
    assert(num_active_fetches_ == 0); // trigger assert on debug
    return;
  }

  curl_multi_cleanup(curl_handle_);
}

// Handles the timer event.
void CurlEngineImpl::on_timeout()
{
  int running_handles = 0;
  auto mcode = curl_multi_socket_action(curl_handle_, CURL_SOCKET_TIMEOUT, 0, &running_handles);
  if (mcode != CURLM_OK) {
    LOG::error("Cannot trigger CURL_SOCKET_TIMEOUT: {}", curl_multi_strerror(mcode));
    throw std::runtime_error("Cannot trigger CURL_SOCKET_TIMEOUT.");
    return;
  }

  handle_curl_info_queue();

  // TODO: we should check if any long running transfers here.
  //       Returns timeout status to the client.
}

// Handles the poll event.
void CurlEngineImpl::on_poll(uv_poll_t *poll_handle, int flags)
{
  int running_handles = 0;
  int fd = 0;
  int res = uv_fileno((const uv_handle_t *)poll_handle, &fd);
  if (res != 0) {
    LOG::error("Cannot extract fd from poll handle. {}", uv_err_name(res));
    throw std::runtime_error("extract fd from poll handle.");
    return;
  }

  auto mcode = curl_multi_socket_action(curl_handle_, fd, flags, &running_handles);

  if (mcode != CURLM_OK) {
    LOG::error("Cannot trigger IN/OUT event: {}", curl_multi_strerror(mcode));
    throw std::runtime_error("Cannot trigger IN/OUT event");
    return;
  }
  handle_curl_info_queue();
}

// Drains and handles messages from Curl info queue.
// Dispatches a callback to client when the transfer is done.
void CurlEngineImpl::handle_curl_info_queue()
{
  int pending = 0;
  CURLMsg *message = nullptr;
  while ((message = curl_multi_info_read(curl_handle_, &pending)) != nullptr) {
    if (message->msg != CURLMSG_DONE) {
      LOG::warn("Unknown curl message {}", message->msg);
      continue;
    }

    CURL *easy_handle = message->easy_handle;
    CurlEngineImpl::FetchRequest *request = nullptr;
    curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &request);
    assert(request != nullptr);

    auto status = CurlEngineStatus::OK;
    std::string_view error_message;
    if (message->data.result == CURLE_OK) {
      LOG::trace_in(Utility::curl, "Successfully fetch from {}", request->target());
      error_message = "successfull fetch";
    } else {
      LOG::debug_in(Utility::curl, "Fail to fetch from {}, {}", request->target(), request->error_message());
      status = CurlEngineStatus::ERROR;
      error_message = request->error_message();
    }
    // Clean up before signal
    assert(num_active_fetches_ != 0);
    num_active_fetches_--;

    if (auto mcode = curl_multi_remove_handle(curl_handle_, easy_handle); mcode != CURLM_OK) {
      LOG::error("Cannot remove a handle: {}", curl_multi_strerror(mcode));
      throw std::runtime_error("Cannot remove a handle.");
      return;
    }

    request->done(status, message->data.result == CURLE_OK, error_message);
  }
}

CurlEngineStatus CurlEngineImpl::schedule_fetch(FetchRequest &request)
{
  LOG::trace_in(Utility::curl, "Schedule a fetch: {}", request.target());

  if (auto mcode = curl_multi_add_handle(curl_handle_, request.prepare()); mcode != CURLM_OK) {
    LOG::error("Cannot schedule fetch: {}", curl_multi_strerror(mcode));
    request.done(CurlEngineStatus::SCHEDULE_ERROR, false, curl_multi_strerror(mcode));
    return CurlEngineStatus::ERROR;
  }

  num_active_fetches_++;
  return CurlEngineStatus::OK;
}

CurlEngineStatus CurlEngineImpl::cancel_fetch(FetchRequest &request)
{
  assert(num_active_fetches_ != 0);

  LOG::trace_in(Utility::curl, "Cancel a fetch: {}", request.target());
  num_active_fetches_--;

  if (auto mcode = curl_multi_remove_handle(curl_handle_, *request); mcode != CURLM_OK) {
    LOG::error("Cannot cancel a fetch: {}", curl_multi_strerror(mcode));
    return CurlEngineStatus::ERROR;
  }

  request.done(CurlEngineStatus::CANCELED, false, "request cancelled");
  return CurlEngineStatus::OK;
}

void CurlEngineImpl::start_timer(u64 timeout_ms)
{
  stop_timer();
  uv_timer_start(&timer_, on_uv_timeout, timeout_ms, 0);
}

void CurlEngineImpl::stop_timer()
{
  uv_timer_stop(&timer_);
}

void CurlEngineImpl::start_poll(curl_socket_t socket_fd, int events)
{
  LOG::trace_in(Utility::curl, "start_poll(), events: {}", events);
  std::unique_ptr<uv_poll_t> poll_handle(new uv_poll_t);

  int res = uv_poll_init(loop_, poll_handle.get(), socket_fd);
  if (res != 0) {
    LOG::error("Cannot init poll handle. {}", uv_err_name(res));
    throw std::runtime_error("Cannot init poll handle.");
    return;
  }
  poll_handle->data = this;

  res = uv_poll_start(poll_handle.get(), events, on_uv_poll);
  if (res != 0) {
    LOG::error("Cannot start poll. {}", uv_err_name(res));
    throw std::runtime_error("Cannot start poll.");
    return;
  }

  auto mcode = curl_multi_assign(curl_handle_, socket_fd, poll_handle.release());
  if (mcode != CURLM_OK) {
    LOG::error("Cannot add easy handle", curl_multi_strerror(mcode));
    throw std::runtime_error("Cannot add easy handle.");
    return;
  }
}

void CurlEngineImpl::remove_poll(uv_poll_t *poll_handle)
{
  LOG::trace_in(Utility::curl, "remove_poll()");

  uv_close((uv_handle_t *)poll_handle, [](uv_handle_t *handle) { delete (uv_poll_t *)handle; });
}

} // namespace
