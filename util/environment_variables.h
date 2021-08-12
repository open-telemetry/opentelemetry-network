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

#include <string>
#include <string_view>

/**
 * This file contains utilities for dealing with environment variables.
 */

/**
 * Reads the environment variable named `name` and returns its value. Name must be
 * a null-terminated string.
 *
 * If the variable is not found then `fallback` is refurned instead (defaults to
 * empty `string_view`).
 *
 * NOTE: this function reads environment variables so it's advisable to call it
 * before any thread is created, given that reading/writing to environment
 * variables is not thread safe and we can't control 3rd party libraries.
 */
std::string_view try_get_env_var(char const *name, std::string_view fallback = {});

/**
 * Reads the environment variable named `name` and returns its value converted
 * to type `T`. Name must be a null-terminated string.
 *
 * If the variable is not found or its value can't be converted to `T` then
 * `fallback` is refurned instead (defaults to value initialized `T`).
 *
 * NOTE: this function reads environment variables so it's advisable to call it
 * before any thread is created, given that reading/writing to environment
 * variables is not thread safe and we can't control 3rd party libraries.
 */
template <typename T> T try_get_env_value(char const *name, T fallback = {});

/**
 * Reads the environment variable named `name` and returns its value. Name must be
 * a null-terminated string.
 *
 * If the variable is not found an `invalid_argument` exception is thrown.
 *
 * NOTE: this function reads environment variables so it's advisable to call it
 * before any thread is created, given that reading/writing to environment
 * variables is not thread safe and we can't control 3rd party libraries.
 */
std::string get_env_var(char const *name);

#include <util/environment_variables.inl>
