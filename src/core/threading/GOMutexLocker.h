/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMUTEXLOCKER_H
#define GOMUTEXLOCKER_H

#include "GOMutex.h"

class GOMutexLocker {
private:
  GOMutex &m_mutex;
  bool m_IsLocked;

public:
  /**
   * Lock the mutex
   * When try_lock = true or pThread is not null then the mutex may be not
   * locked after invoking and the caller must check with IsLocked(). Otherwise
   * the mutex is locked after return and the caller may not worry about it
   *
   * @param mutex: mutex to lock
   * @param try_lock: if true and the mutex is ocuppied then returns immediate
   * @param lockerInfo an information who locks the mutex. Useful for diagnostic
   * deadlocks
   * @param pThread. If != NULL then the constructor may return wihout locking
   * when pThread->ShouldStop()
   */

  GOMutexLocker(
    GOMutex &mutex,
    bool try_lock = false,
    const char *lockerInfo = nullptr,
    GOThread *pThread = nullptr);

  ~GOMutexLocker();

  bool TryLock(const char *lockerInfo = nullptr);

  bool IsLocked() const { return m_IsLocked; }

  void Unlock();

  GOMutexLocker(const GOMutexLocker &) = delete;
  GOMutexLocker &operator=(const GOMutexLocker &) = delete;
};

#endif
