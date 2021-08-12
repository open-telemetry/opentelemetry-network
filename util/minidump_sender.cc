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

#include <util/minidump_sender.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Form.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Types.hpp>

bool MinidumpSender::send(
    std::string const &url,
    std::list<std::string> headers,
    std::map<std::string, std::string> const &parameters,
    std::map<std::string, std::string> const &files,
    std::string const &proxy,
    std::string const &proxy_user_pwd,
    std::string const &ca_certificate_file)
{
  curlpp::Easy curl;

  curl.setOpt(curlpp::Options::Url(url));
  curl.setOpt(curlpp::Options::UserAgent("FlowmillMinidump/1.0 (Linux)"));

  /* comment from http_upload.cc: */
  // Support multithread by disabling timeout handling, would get SIGSEGV with
  // Curl_resolv_timeout in stack trace otherwise.
  // See https://curl.haxx.se/libcurl/c/threadsafe.html
  curl.setOpt(curlpp::Options::NoSignal(1));

  if (!proxy.empty()) {
    curl.setOpt(curlpp::Options::Proxy(proxy));
  }
  if (!proxy_user_pwd.empty()) {
    curl.setOpt(curlpp::Options::ProxyUserPwd(proxy_user_pwd));
  }

  if (!ca_certificate_file.empty()) {
    curl.setOpt(curlpp::Options::CaInfo(ca_certificate_file));
  }

  // Disable 100-continue header.
  headers.emplace_back("Expect:");
  curl.setOpt(curlpp::Options::HttpHeader(headers));

  /* set POST from parameters and files */
  curlpp::Forms form_parts;
  for (auto const &it : parameters) {
    form_parts.push_back(new curlpp::FormParts::Content(it.first, it.second));
  }
  for (auto const &it : files) {
    form_parts.push_back(new curlpp::FormParts::File(it.first, it.second));
  }
  curl.setOpt(curlpp::Options::HttpPost(form_parts));

  /* if we need to get response body */
  curl.setOpt(curlpp::Options::WriteFunction([this](char *ptr, size_t size, size_t nmemb) -> size_t {
    size_t real_size = size * nmemb;
    response_body_.append(reinterpret_cast<char *>(ptr), real_size);
    return real_size;
  }));

  // Fail if 400+ is returned from the web server.
  curl.setOpt(curlpp::Options::FailOnError(1));

  try {
    curl.perform();
  } catch (curlpp::LibcurlRuntimeError const &e) {
    response_code_ = e.whatCode();
    error_description_ = std::string(e.what());
    return false;
  }

  return true;
}
