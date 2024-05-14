/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOALLOC_H
#define GOALLOC_H

#include <memory>

class GOOutOfMemory {};

template <class T, class... Args> std::unique_ptr<T> GOAlloc(Args... args) {
  try {
    return std::unique_ptr<T>(new T(args...));
  } catch (std::bad_alloc &ba) {
    throw GOOutOfMemory();
  }
}

template <class T> std::unique_ptr<T[]> GOAllocArray(size_t count) {
  try {
    return std::unique_ptr<T[]>(new T[count]);
  } catch (std::bad_alloc &ba) {
    throw GOOutOfMemory();
  }
}

#endif
