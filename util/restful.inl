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

#include <util/log.h>
#include <util/log_formatters.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

template <typename T> Expected<T, std::runtime_error> RestfulFetcher::CtorDecoder<T>::operator()(std::string body) const
{
  try {
    return T{std::move(body)};
  } catch (std::runtime_error const &e) {
    return {unexpected, e};
  } catch (std::exception const &e) {
    return {unexpected, std::runtime_error{e.what()}};
  }
}

template <typename T, typename Decoder>
Expected<T, std::runtime_error> RestfulFetcher::sync_fetch(
    std::string_view description,
    std::string url,
    Decoder &&decoder,
    std::chrono::milliseconds const timeout,
    std::size_t const retries,
    std::chrono::milliseconds const initial_backoff,
    std::chrono::milliseconds const maximum_backoff,
    unsigned const backoff_geometric_ratio)
{
  curlpp::Easy request;

  request.setOpt<curlpp::options::Url>(std::move(url));
  request.setOpt<curlpp::options::HttpHeader>(headers_);

  if (!proxy_.empty()) {
    request.setOpt<curlpp::options::Proxy>(proxy_);
    request.setOpt<curlpp::options::ProxyPort>(proxy_port_);
    request.setOpt<curlpp::options::ProxyType>(proxy_type_);
  }

  if (timeout.count()) {
    request.setOpt<curlpp::OptionTrait<long, CURLOPT_TIMEOUT_MS>>(timeout.count());
  }

  std::ostringstream body;
  request.setOpt<curlpp::options::WriteStream>(&body);

  auto backoff_interval = initial_backoff;

  for (std::size_t attempt = 0; attempt <= retries; ++attempt) {
    bool const last_attempt = attempt == retries;

    auto const backoff = [&](std::string_view error) {
      if (last_attempt) {
        return false;
      }

      LOG::warn("backing-off for {} before trying again after failing to fetch {}: {}", backoff_interval, description, error);

      std::this_thread::sleep_for(backoff_interval);

      backoff_interval = std::min(backoff_interval * backoff_geometric_ratio, maximum_backoff);

      return true;
    };

    try {
      request.perform();

      auto const status = curlpp::infos::ResponseCode::get(request);

      if (status != 200) {
        auto error = fmt::format("error while fetching {}: status code {}", description, status);
        if (!backoff(error)) {
          return {unexpected, std::move(error)};
        }
        continue;
      }

      auto decoded = decoder(body.str());
      if (!decoded) {
        auto error = fmt::format("error while decoding {}: {}", description, decoded.error());
        if (!backoff(error)) {
          return {unexpected, std::move(error)};
        }
        continue;
      }

      return std::move(*decoded);
    } catch (curlpp::LibcurlRuntimeError const &e) {
      auto error = fmt::format("error while fetching {}: {}", description, e.what());
      if (!backoff(error)) {
        return {unexpected, std::move(error)};
      }
      continue;
    }
  }

  return {unexpected, fmt::format("unable to fetch {} after {} tries", description, retries + 1)};
}
