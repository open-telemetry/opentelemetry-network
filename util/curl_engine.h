/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>
#include <util/enum.h>

#include <curl/curl.h>
#include <uv.h>

#include <functional>
#include <initializer_list>
#include <memory>
#include <string_view>

#define ENUM_NAME CurlEngineStatus
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(OK, 0)                                                                                                                     \
  X(TIMEOUT, 1)                                                                                                                \
  X(ERROR, 2)                                                                                                                  \
  X(CANCELED, 3)                                                                                                               \
  X(SCHEDULE_ERROR, 4)
#define ENUM_DEFAULT ERROR
#include <util/enum_operators.inl>

// CurlEngine utilizes libcurl's multi socket API on top of libuv event loop to
// fetch data from HTTP endpoints.
//
// It is expected that both CurlEngine and its clients are running on top of
// same libuv loop.
class CurlEngine {
public:
  CurlEngine() = default;
  virtual ~CurlEngine() = default;

  // Factory function to create CurlEngine.
  // The caller take the ownership of the returned value.
  static std::unique_ptr<CurlEngine> create(uv_loop_t *loop);

  // Callback function to be executed when some data has been fetched
  // and is available to consumed.
  //
  // This callback might be invoked multiple times, as more data is returned.
  //
  // |data|: returned data. The calllback does not take the ownership of |data|
  // |data_length|: number of bytes of |data|.
  //
  // CurlEngine might overwrite |data| after this callback is invoked. Thus,
  // it's client's responsiblity to make a copy of |data| if that's needed.
  using DataAvailableFn = std::function<void(const char *data, size_t data_length)>;

  // Callback function to be  executed when fetch finishes.
  // |status|: the status of the fetch.
  // |resonseCode|: the response code of the fetch, or -1 if not available.
  //
  // It is possible that DataAvailableFn is invoked multiple times,
  // before a FetchDoneFn is invoked with status != CurlEngineStatus::OK.
  //
  // The associated FetchRequest is unregistered from the CurlEngine before
  // this callback is invoked. The client is free to clean up the FetchRequest.
  // curlError.data() is only set when status != OK, and can be used as a
  // null-terminated string.
  using FetchDoneFn = std::function<void(CurlEngineStatus status, long responseCode, std::string_view curlError)>;

  struct FetchRequest {
    // |target|:  the target URL we are going to fetch.
    // |available_fn|: callback when data is available.
    // |done_fn|: callback when fetch finishes.
    FetchRequest(std::string target, DataAvailableFn available_fn, FetchDoneFn done_fn);

    ~FetchRequest();

    template <typename T> bool try_set_option(CURLoption option, T &&value)
    {
      return curl_easy_setopt(easy_handle_, option, std::forward<T>(value)) == CURLE_OK;
    }

    template <typename T> FetchRequest &set_option(CURLoption option, T &&value)
    {
      try_set_option(option, std::forward<T>(value));
      return *this;
    }

    template <typename T> T *get_info(CURLINFO info, T &fallback)
    {
      return curl_easy_getinfo(easy_handle_, info, &fallback) == CURLE_OK ? &fallback : nullptr;
    }

    // adds the header in the request
    bool add_header(char const *header);

    void http_proxy(std::string host, std::uint16_t port);

    // send the request through a UNIX socket
    void unix_socket(std::string_view path);
    void debug_mode(bool debug);

    std::string_view target() { return target_; }
    std::string_view error_message() { return error_message_; }

    void done(CurlEngineStatus status, bool success, std::string_view error);

    CURL const *operator*() const { return easy_handle_; }
    CURL *operator*() { return easy_handle_; }

    bool operator!() const { return !easy_handle_; }
    explicit operator bool() const { return static_cast<bool>(easy_handle_); }

    CURL *prepare();

  private:
    CURL *easy_handle_ = nullptr;
    curl_slist *headers_ = nullptr;
    std::string target_;
    DataAvailableFn available_fn_;
    FetchDoneFn done_fn_;

    std::string unix_socket_;

    std::string proxy_;
    std::uint16_t proxy_port_;
    curl_proxytype proxy_type_;

    char error_message_[CURL_ERROR_SIZE] = {0};

    static size_t curl_write(char *ptr, size_t size, size_t nmemb, void *userdata);
  };

  // Schedules to fetch.
  // |request|: fetch request, it should be created by CreateFetchRequest().
  //
  // The done_fn callback from |request| is invoked immedidate if error occurs.
  // The CurlEngine does not take ownership of |request|.
  //
  virtual CurlEngineStatus schedule_fetch(FetchRequest &request) = 0;

  // Stops and cancels the ongoing fetch.
  //
  //
  // The FetchDoneFn callback with be invoked with CurlEngineStatus::CANCELED status.
  //
  // Client must explicitly cancel the FetchRequest, (or waits until the
  // request finishes), before destroy the FeetchRequest object.
  //
  // It will be a no-op if the request has not been scheduled by the CurlEngine,
  // or it is scheduled and finishes (FetchDoneFn callback has been invoked).
  virtual CurlEngineStatus cancel_fetch(FetchRequest &request) = 0;
};
