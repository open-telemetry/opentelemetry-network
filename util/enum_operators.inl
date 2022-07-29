// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

//
// enum_operators.inl
//
// define an enum that has a set of useful overloaded operators
// automatically defines:
//      1. the enum
//      2. to_string/enum_from_string/try_enum_from_string
//      3. sanitize_enum convenience function
//      4. enum_traits for the enum
//      5. operator<<
//
// to use:
//
// #include <util/enum.h>
//
// #define ENUM_NAME Foobar
// #define ENUM_TYPE uint32_t
// #define ENUM_ELEMENTS(X) \.
//            X(key_a, 0)   \.
//            X(key_b, 1)   \.
//            X(key_c, 2)   \.
//            ...
// #define ENUM_DEFAULT key_a
// #include <util/enum_operators.inl>
//
// There is no need to #undef these macros as they are #undef'd in this file
//

#include <util/preprocessor.h>

#ifndef ENUM_NAME
#error "You must declare the name of the enum in ENUM_NAME"
#endif

#ifndef ENUM_TYPE
#define ENUM_TYPE unsigned
#endif

#ifndef ENUM_ELEMENTS
#error "You must declare the list of enum element keys and values in ENUM_ELEMENTS"
#endif

#ifdef ENUM_NAMESPACE
namespace ENUM_NAMESPACE {
#define ENUM_FULLY_QUALIFIED_NAME ::ENUM_NAMESPACE::ENUM_NAME
#else // ENUM_NAMESPACE
#define ENUM_FULLY_QUALIFIED_NAME ::ENUM_NAME
#endif // ENUM_NAMESPACE

// Declare the enum itself
#define ENUM_ELEMENT(K, V) K = V,
enum class ENUM_NAME : ENUM_TYPE { ENUM_ELEMENTS(ENUM_ELEMENT) };
#undef ENUM_ELEMENT

#ifdef ENUM_NAMESPACE
} // namespace ENUM_NAMESPACE
#endif // ENUM_NAMESPACE

template <> struct enum_traits<ENUM_FULLY_QUALIFIED_NAME> {
  using type = ENUM_FULLY_QUALIFIED_NAME;
  using int_type = std::underlying_type_t<type>;

#ifdef ENUM_DEFAULT
  static constexpr type default_value = type::ENUM_DEFAULT;
#endif // ENUM_DEFAULT

  static constexpr bool has_default_value =
#ifdef ENUM_DEFAULT
      true
#else  // ENUM_DEFAULT
      false
#endif // ENUM_DEFAULT
      ;

  static constexpr type min()
  {
    constexpr std::initializer_list<type> values = {
#define LIST_ENUM_VALUES(K, V) type::K,
        ENUM_ELEMENTS(LIST_ENUM_VALUES)
#undef LIST_ENUM_VALUES
    };
    return std::min(values);
  }

  static constexpr type max()
  {
    constexpr std::initializer_list<type> values = {
#define LIST_ENUM_VALUES(K, V) type::K,
        ENUM_ELEMENTS(LIST_ENUM_VALUES)
#undef LIST_ENUM_VALUES
    };
    return std::max(values);
  }

  static constexpr int_type count = [] {
    return
#define COUNT_ENUM_VALUES(K, V) 1 +
        ENUM_ELEMENTS(COUNT_ENUM_VALUES)
#undef COUNT_ENUM_VALUES
            0;
  }();

  static constexpr bool is_valid(type value)
  {
    switch (value) {
#define SWITCH_ENUM_TYPES(K, V)                                                                                                \
  case type::K:                                                                                                                \
    return true;
      ENUM_ELEMENTS(SWITCH_ENUM_TYPES)
#undef SWITCH_ENUM_TYPES
    }
    return false;
  }

  static constexpr std::array<type, count> values{
#define LIST_ENUM_VALUES(K, V) type::K,
      ENUM_ELEMENTS(LIST_ENUM_VALUES)
#undef LIST_ENUM_VALUES
  };

  static constexpr bool is_contiguous = [] {
    for (std::size_t i = 1; i < values.size(); ++i) {
      if (static_cast<int_type>(values[i]) != static_cast<int_type>(values[i - 1]) + 1) {
        return false;
      }
    }
    return true;
  }();

  /**
   * Returns an array that can be indexed with this enum.
   *
   * Converting the enum values to indexes into this array can be accomplished
   * by calling `enum_index_of(value)`.
   */
  template <typename T> using array_map = std::array<T, count>;
};

#ifdef ENUM_NAMESPACE
namespace ENUM_NAMESPACE {
#endif // ENUM_NAMESPACE

// String conversion operations
constexpr inline std::string_view to_string(
    ENUM_NAME value,
#ifdef ENUM_DEFAULT
    std::string_view fallback = PREPROC_STRINGIZE(ENUM_DEFAULT)
#else  // ENUM_DEFAULT
    std::string_view fallback = {}
#endif // ENUM_DEFAULT
)
{
  switch (value) {
#define SWITCH_ENUM_TYPES(K, V)                                                                                                \
  case ENUM_NAME::K:                                                                                                           \
    return #K;
    ENUM_ELEMENTS(SWITCH_ENUM_TYPES)
#undef SWITCH_ENUM_TYPES
  }
  return fallback;
}

constexpr inline bool enum_from_string(std::string_view s, ENUM_NAME &out)
{
#define IF_ENUM_TYPES(K, V)                                                                                                    \
  if (s == #K) {                                                                                                               \
    out = ENUM_NAME::K;                                                                                                        \
    return true;                                                                                                               \
  }
  ENUM_ELEMENTS(IF_ENUM_TYPES)
#undef IF_ENUM_TYPES
  return false;
}

constexpr inline ENUM_NAME try_enum_from_string(
    std::string_view s,
#ifdef ENUM_DEFAULT
    ENUM_NAME fallback = ENUM_NAME::ENUM_DEFAULT
#else  // ENUM_DEFAULT
    ENUM_NAME fallback
#endif // ENUM_DEFAULT
)
{
  enum_from_string(s, fallback);
  return fallback;
}

// Sanitizing convenience function
constexpr inline ENUM_NAME sanitize_enum(
    ENUM_NAME value,
    ENUM_NAME default_value
#ifdef ENUM_DEFAULT
    = ENUM_NAME::ENUM_DEFAULT
#endif // ENUM_DEFAULT
)
{
  return enum_traits<ENUM_NAME>::is_valid(value) ? value : default_value;
}

// Stream out overload
template <typename Out> Out &operator<<(Out &&out, ENUM_NAME value)
{
  out << to_string(value);
  return out;
}

#ifdef ENUM_NAMESPACE
} // namespace ENUM_NAMESPACE
#endif // ENUM_NAMESPACE

// Clean up the things we've defined
#undef ENUM_NAME
#undef ENUM_FULLY_QUALIFIED_NAME
#undef ENUM_TYPE
#undef ENUM_ELEMENTS
#ifdef ENUM_DEFAULT
#undef ENUM_DEFAULT
#endif // ENUM_DEFAULT
#ifdef ENUM_NAMESPACE
#undef ENUM_NAMESPACE
#endif // ENUM_NAMESPACE
