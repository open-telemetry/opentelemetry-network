/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cassert>

// This file contains a handful of utilities for parsing a collection of chars.
// ITERATOR cur is the current position in the collection, while ITERATOR end 
// is the end of the collection.
//
// ITERATOR cur needs to be at least a forward iterator.
// peek() and lookahead() do not modify cur, but the consume(), parse_match() 
// and parse_token() functions do.
//
// FUTURE - would it be worth templating the all the 'char' to 
// std::iterator_traits<ITERATOR>::value_type ?
namespace parsing {

// get the next character into c, but don't consume it
template <typename ITERATOR> 
inline bool peek(ITERATOR cur, ITERATOR end, char *c) {
  if (cur == end) { 
    return false;
  }

  *c = *cur;

  return true;
}

// see if the next character matches c
template <typename ITERATOR> 
inline bool lookahead(ITERATOR cur, ITERATOR end, char c) {
  char f;
  if (!peek(cur, end, &f)) return false;

  return f == c;
}

// see if the next character matches the predicate
template <typename ITERATOR> 
inline bool lookahead(ITERATOR cur, ITERATOR end, bool (*pred)(char))
{
  char f;
  if (!peek(cur, end, &f)) return false;

  return pred(f);
}

// look at the next characters to see if they match string s
template <typename ITERATOR> 
inline bool lookahead(ITERATOR cur, ITERATOR end, std::string_view s)
{
  auto dist = std::distance(cur, end);
  assert(dist >= 0);
  if ((size_t)dist < s.size()) return false;

  ITERATOR t = cur;
  for (size_t ii = 0; ii < s.size(); ++ii, ++t)
  {
    if (*t != s[ii]) {
      return false;
    }
  }

  return true;
}

// eat the next n characters (default 1)
template <typename ITERATOR> 
inline void consume(ITERATOR &cur, ITERATOR end, size_t n = 1)
{
  for (size_t ii = 0; ii < n && cur != end; ++ii)
  {
    ++cur;
  }
}

// compare the front character to c and consume if matches,
// optionally storing it in s
template <typename ITERATOR>
inline bool parse_match(ITERATOR &cur, ITERATOR end, char c, std::string *s = nullptr)
{
  if (lookahead(cur, end, c))
  {
    consume(cur, end);
    if (s != nullptr)
    {
      (*s) += c;
    }

    return true;
  }

  return false;
}

// check, leave untouched if failed, otherwise consume and optionally store
// at out
template <typename ITERATOR>
inline bool parse_match(ITERATOR &cur, ITERATOR end, std::string_view s, std::string *out = nullptr)
{
  if (lookahead(cur, end, s))
  {
    consume(cur, end, s.size());
    if (out != nullptr)
    {
      (*out) += s;
    }

    return true;
  }

  return false;
}

// compare the front character to a predicate and consume if matches,
// optionally storing it in s
template <typename ITERATOR>
inline bool parse_match(ITERATOR &cur, ITERATOR end, bool (*pred)(char), std::string *s = nullptr)
{
  if (lookahead(cur, end, pred))
  {
    if (s != nullptr)
    {
      (*s) += *cur;
    }
    consume(cur, end);
    return true;
  }

  return false;
}

// consume until token is reached, optionally storing consumed in s.
// NB: it does consume the token, but does NOT store it in s.
template <typename ITERATOR>
inline bool parse_token(ITERATOR &cur, ITERATOR end, char token, std::string *s = nullptr) {
  char p;
  while (peek(cur, end, &p)) {
    consume(cur, end);
    if (p == token) {
      return true;
    }

    if (s != nullptr) {
      (*s) += p;
    }
  }

  return false;
}

} // namespace parsing
