// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/base64.h>

#include <openssl/bio.h>
#include <openssl/evp.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

std::string base64_encode(std::string_view input)
{
  std::string buffer;
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

  return buffer;
}
