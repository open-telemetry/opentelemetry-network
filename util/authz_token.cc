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

#include <util/authz_token.h>

#include <util/json.h>
#include <util/log.h>

#include <optional>

#include <cstdlib>

namespace {
static std::string const TOKEN_KEY = "token";
static std::string const ISSUED_AT_KEY = "issuedAtS";
static std::string const EXPIRATION_KEY = "expirationS";
static std::string const INTAKE_KEY = "intake";
} // namespace

AuthzToken::AuthzToken(std::string payload, duration issued_at, duration expiration, std::string intake)
    : payload_(std::move(payload)), issued_at_(issued_at), expiration_(expiration), intake_(std::move(intake))
{}

Expected<AuthzToken, std::runtime_error> AuthzToken::decode_json(std::string_view json)
{
  auto const decoded = [json]() -> Expected<nlohmann::json, std::runtime_error> {
    try {
      return nlohmann::json::parse(json);
    } catch (std::exception const &e) {
      auto error = fmt::format("invalid authz token encoding format: {} - json: '{}'", e.what(), json);
      LOG::error(error);
      return {unexpected, std::move(error)};
    }
  }();

  if (!decoded) {
    return {unexpected, std::move(decoded.error())};
  }

  auto const token = try_get_string(*decoded, TOKEN_KEY);
  auto const issued_at = try_get_as_int(*decoded, ISSUED_AT_KEY);
  auto const expiration = try_get_as_int(*decoded, EXPIRATION_KEY);
  auto const intake = try_get_string(*decoded, INTAKE_KEY);
  if (!token || !issued_at || !expiration || *expiration < *issued_at || !intake) {
    auto error = "missing metadata from authz token response";
    LOG::error(error);
    return {unexpected, std::move(error)};
  }

  return {*token, std::chrono::seconds(*issued_at), std::chrono::seconds(*expiration), *intake};
}
