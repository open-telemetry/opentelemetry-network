/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <string_view>

std::string format_url(std::string host, std::string_view path, std::string_view default_scheme = "https");
