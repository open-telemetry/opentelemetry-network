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

#pragma once

#include <util/enum.h>

#include <type_traits>

#include <cstdint>

#define ENUM_NAME HttpStatusCode
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
                                                                                                                               \
  /* informational */                                                                                                          \
  X(Continue, 100)                                                                                                             \
  X(SwitchingProtocols, 101)                                                                                                   \
  X(Processing, 102)                                                                                                           \
  X(EarlyHints, 103)                                                                                                           \
                                                                                                                               \
  /* success */                                                                                                                \
  X(OK, 200)                                                                                                                   \
  X(Created, 201)                                                                                                              \
  X(Accepted, 202)                                                                                                             \
  X(NonAuthoritativeInformation, 203)                                                                                          \
  X(NoContent, 204)                                                                                                            \
  X(ResetContent, 205)                                                                                                         \
  X(PartialContent, 206)                                                                                                       \
  X(MultiStatus, 207)                                                                                                          \
  X(AlreadyReported, 208)                                                                                                      \
  X(InstanceManipulationUsed, 226)                                                                                             \
                                                                                                                               \
  /* redirection */                                                                                                            \
  X(MultipleChoices, 300)                                                                                                      \
  X(MovedPermanently, 301)                                                                                                     \
  X(Found, 302)                                                                                                                \
  X(SeeOther, 303)                                                                                                             \
  X(NotModified, 304)                                                                                                          \
  X(UseProxy, 305)                                                                                                             \
  X(SwitchProxy, 306)                                                                                                          \
  X(TemporaryRedirect, 307)                                                                                                    \
  X(PermanentRedirect, 308)                                                                                                    \
                                                                                                                               \
  /* client error */                                                                                                           \
  X(BadRequest, 400)                                                                                                           \
  X(Unauthorized, 401)                                                                                                         \
  X(PaymentRequired, 402)                                                                                                      \
  X(Forbidden, 403)                                                                                                            \
  X(NotFound, 404)                                                                                                             \
  X(MethodNotAllowed, 405)                                                                                                     \
  X(NotAcceptable, 406)                                                                                                        \
  X(ProxyAuthenticationRequired, 407)                                                                                          \
  X(RequestTimeout, 408)                                                                                                       \
  X(Conflict, 409)                                                                                                             \
  X(Gone, 410)                                                                                                                 \
  X(LengthRequired, 411)                                                                                                       \
  X(PreconditionFailed, 412)                                                                                                   \
  X(PayloadTooLarge, 413)                                                                                                      \
  X(URITooLong, 414)                                                                                                           \
  X(UnsupportedMediaType, 415)                                                                                                 \
  X(RangeNotSatisfiable, 416)                                                                                                  \
  X(ExpectationFailed, 417)                                                                                                    \
  X(ImATeapot, 418)                                                                                                            \
  X(MisdirectedRequest, 421)                                                                                                   \
  X(UnprocessableEntity, 422)                                                                                                  \
  X(Locked, 423)                                                                                                               \
  X(FailedDependency, 424)                                                                                                     \
  X(TooEarly, 425)                                                                                                             \
  X(UpgradeRequired, 426)                                                                                                      \
  X(PreconditionRequired, 428)                                                                                                 \
  X(TooManyRequests, 429)                                                                                                      \
  X(RequestHeaderFieldsTooLarge, 431)                                                                                          \
  X(UnavailableForLegalReasons, 451)                                                                                           \
                                                                                                                               \
  /* server error */                                                                                                           \
  X(InternalServerError, 500)                                                                                                  \
  X(NotImplemented, 501)                                                                                                       \
  X(BadGateway, 502)                                                                                                           \
  X(ServiceUnavailable, 503)                                                                                                   \
  X(GatewayTimeout, 504)                                                                                                       \
  X(HTTPVersionNotSupported, 505)                                                                                              \
  X(VariantAlsoNegotiates, 506)                                                                                                \
  X(InsufficientStorage, 507)                                                                                                  \
  X(LoopDetected, 508)                                                                                                         \
  X(NotExtended, 510)                                                                                                          \
  X(NetworkAuthenticationRequired, 511)
#include <util/enum_operators.inl>

constexpr bool is_informational_class(HttpStatusCode code)
{
  auto const value = static_cast<std::underlying_type_t<HttpStatusCode>>(code);
  return value >= 100 && value < 200;
}

constexpr bool is_success_class(HttpStatusCode code)
{
  auto const value = static_cast<std::underlying_type_t<HttpStatusCode>>(code);
  return value >= 200 && value < 300;
}

constexpr bool is_redirection_class(HttpStatusCode code)
{
  auto const value = static_cast<std::underlying_type_t<HttpStatusCode>>(code);
  return value >= 300 && value < 400;
}

constexpr bool is_client_error_class(HttpStatusCode code)
{
  auto const value = static_cast<std::underlying_type_t<HttpStatusCode>>(code);
  return value >= 400 && value < 500;
}

constexpr bool is_server_error_class(HttpStatusCode code)
{
  auto const value = static_cast<std::underlying_type_t<HttpStatusCode>>(code);
  return value >= 500 && value < 600;
}
