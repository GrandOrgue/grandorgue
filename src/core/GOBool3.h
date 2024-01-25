/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBOOL3_H
#define GOBOOL3_H

#include <cstdint>

// 3-value booleans
enum GOBool3 : int8_t {
  BOOL3_DEFAULT = -1,
  BOOL3_FALSE = 0, // must be the same as ``(int8_t) false``
  BOOL3_TRUE = 1,  // must be the same as ``(int8_t) true``
  BOOL3_MIN = BOOL3_DEFAULT,
  BOOL3_MAX = BOOL3_TRUE
};

// conversion from any number and bool
template <typename T> GOBool3 to_bool3(T from) { return (GOBool3)(int8_t)from; }

#define CONVERT_BOOL3_TO_BOOL(fromBool3, defaultBoolExpr)                      \
  fromBool3 >= BOOL3_FALSE ? (bool)(int8_t)(fromBool3) : defaultBoolExpr

// conversion with lazy calculation of default
template <typename F> inline bool to_bool(GOBool3 from, F defaultFunc) {
  return CONVERT_BOOL3_TO_BOOL(from, defaultFunc());
}

// conversion with a simple default
inline bool to_bool(GOBool3 from, bool defaultValue = false) {
  /*
    we don't implement it as
    ``return to_bool(from, [&]() { return defaultValue; });``
    because the performance reason: gcc does't inline calling a trivial lambda
   */
  return CONVERT_BOOL3_TO_BOOL(from, defaultValue);
}

#undef CONVERT_BOOL3_TO_BOOL

#endif /* GOBOOL3_H */
