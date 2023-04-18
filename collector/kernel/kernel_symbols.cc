// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "kernel_symbols.h"

#include <util/string_view.h>

#include <fstream>
#include <stdexcept>
#include <string>

namespace {

inline std::string_view parse_symbol_name(std::string_view s)
{
  static constexpr std::string_view delimiters = " \t";

  // Skip two tokens (address and type).
  for (size_t i = 0; i < 2; ++i) {
    auto p = s.find_first_of(delimiters);

    if (p == std::string_view::npos) {
      return std::string_view();
    }

    s = views::ltrim_ws(s.substr(p));
  }

  // Return the next token (symbol name).
  return s.substr(0, s.find_first_of(delimiters));
}

} // namespace

KernelSymbols read_proc_kallsyms(std::istream &stream)
{
  KernelSymbols symbols;

  while (stream.good()) {
    std::string line;
    std::getline(stream, line);

    if (line.empty()) {
      continue;
    }

    std::string_view symbol = parse_symbol_name(line);
    if (symbol.empty()) {
      throw std::runtime_error("parse error");
    }

    symbols.insert(std::string(symbol));
  }

  return symbols;
}

KernelSymbols read_proc_kallsyms(const char *path)
{
  std::ifstream file(path);

  if (!file.is_open()) {
    throw std::system_error(errno, std::generic_category(), "error opening file");
  }

  return read_proc_kallsyms(file);
}
