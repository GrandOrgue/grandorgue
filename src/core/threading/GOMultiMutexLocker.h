/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMULTIMUTEXLOCKER_H
#define GOMULTIMUTEXLOCKER_H

#include <vector>

#include "GOMutex.h"

class GOMultiMutexLocker {
private:
  std::vector<GOMutex *> m_Mutex;

public:
  GOMultiMutexLocker() : m_Mutex(0) {}

  ~GOMultiMutexLocker() {
    while (m_Mutex.size()) {
      m_Mutex.back()->Unlock();
      m_Mutex.pop_back();
    }
  }

  void Add(GOMutex &mutex) {
    mutex.Lock();
    m_Mutex.push_back(&mutex);
  }

  GOMultiMutexLocker(const GOMultiMutexLocker &) = delete;
  GOMultiMutexLocker &operator=(const GOMultiMutexLocker &) = delete;
};

#endif /* GOMULTIMUTEXLOCKER_H */
