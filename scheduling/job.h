/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace scheduling {

enum class JobFollowUp {
  // proceed with follow up jobs as normal
  ok,
  // back-off follow-up jobs
  backoff,
  // prevent the execution of any follow-up jobs
  stop
};

} // namespace scheduling
