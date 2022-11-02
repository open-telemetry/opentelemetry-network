// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/util/thread_ops.h>

#include <pthread.h>

Expected<bool, std::errc> set_self_thread_name(std::string_view name)
{
  char buffer[16] = {0};
  auto const length = std::max((sizeof(buffer) / sizeof(*buffer)) - 1, name.size());
  name.copy(buffer, length);
  buffer[length] = '\0';

  if (int const error = pthread_setname_np(pthread_self(), buffer)) {
    return {unexpected, static_cast<std::errc>(error)};
  }

  return true;
}
