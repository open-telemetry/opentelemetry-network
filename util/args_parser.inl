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

#include <util/enum.h>
#include <util/string.h>

#include <spdlog/fmt/fmt.h>

namespace cli {

template <typename T>
ArgsParser::ArgProxy<T>::ArgProxy(
    args::ArgumentParser &parser, std::string const &name, std::string const &description, T const &default_value)
    : arg_(parser, name, description, {name}, [&]() -> std::conditional_t<std::is_enum_v<T>, std::string, T const &> {
        if constexpr (std::is_enum_v<T>) {
          if constexpr (enum_traits<T>::has_default_value) {
            return std::string(to_string(default_value));
          } else {
            return {};
          }
        } else {
          return default_value;
        }
      }())
{}

template <typename T> bool ArgsParser::ArgProxy<T>::given() const
{
  return arg_.Matched();
}

template <typename T> typename ArgsParser::ArgProxy<T>::proxy_ref ArgsParser::ArgProxy<T>::operator*()
{
  if constexpr (std::is_enum_v<T>) {
    auto const &arg = arg_.Get();
    if constexpr (enum_traits<T>::has_default_value) {
      if (arg.empty()) {
        return enum_traits<T>::default_value;
      }
    }
    T value;
    if (!enum_from_string(arg, value)) {
      throw std::invalid_argument(fmt::format("invalid value given for enumeration: '{}'", arg));
    }
    return value;
  } else {
    return arg_.Get();
  }
}

template <typename T> template <typename> T *ArgsParser::ArgProxy<T>::operator->()
{
  return &arg_.Get();
}

template <typename T>
ArgsParser::ArgProxy<T>
ArgsParser::add_arg(std::string const &name, std::string const &description, char const *env_var, T default_value)
{
  if (env_var) {
    if (auto const value = std::getenv(env_var); value && !from_string(value, default_value) && *value) {
      throw std::invalid_argument(fmt::format(
          "unable to convert environment variable '{}' to the type"
          " of command line argument '{}' (value='{}')",
          env_var,
          name,
          value));
    }
  }

  return ArgProxy(parser_, name, description, default_value);
}

template <class HandlerType, typename... Args> HandlerType &ArgsParser::new_handler(Args &&... args)
{
  handlers_.emplace_back(std::make_unique<HandlerType>(*this, std::forward<Args>(args)...));
  return static_cast<HandlerType &>(*handlers_.back().get());
}

} // namespace cli
