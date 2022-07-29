/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
