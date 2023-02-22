// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

/**
 * Error code lookup tool
 *
 * This is a handy utility that can be used to error messages corresponding
 * to a given error code.
 *
 * The lookup is performed across a variety of libraries used internally.
 */

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <uv.h>

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "usage: error_lookup errno_1 [errno_2 [... [errno_N]]]" << std::endl;
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; ++i) {
    auto const code = std::atoi(argv[i]);
    std::cout << "code: " << code << '\n';

    std::cout << "errno: " << std::strerror(code) << '\n';
    std::cout << "libuv: " << uv_err_name(code) << " - " << uv_strerror(code) << '\n';

    // add any other libraries we use here

    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}
