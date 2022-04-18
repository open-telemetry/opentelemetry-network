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

#include <config.h>

#include <util/base64.h>

#include <openssl/bio.h>
#include <openssl/evp.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

std::string base64_encode(std::string_view input)
{
  std::string buffer;
#if !BUILD_WITH_OTLP // work around otlp development custom benv openssl issues
  buffer.resize(4 * std::ceil(static_cast<double>(input.size()) / 3) + 1);

  auto out = ::fmemopen(buffer.data(), buffer.size(), "w");
  auto encoder = BIO_push(BIO_new(BIO_f_base64()), BIO_new_fp(out, BIO_NOCLOSE));

  BIO_set_flags(encoder, BIO_FLAGS_BASE64_NO_NL);

  BIO_write(encoder, input.data(), input.size());
  BIO_flush(encoder);

  BIO_free_all(encoder);
  std::fclose(out);

  if (!buffer.empty() && buffer.back() == '\0') {
    buffer.pop_back();
  }
#endif
  return buffer;
}
