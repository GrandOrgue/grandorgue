/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMUTEX_H
#define GOMUTEX_H

#include <atomic>
#include <mutex>

#include "GOThread.h"

class GOMutex {
private:
  std::timed_mutex m_mutex;

  std::atomic<const char *> m_LockerInfo;

  GOMutex(const GOMutex &) = delete;
  const GOMutex &operator=(const GOMutex &) = delete;

  bool DoLock(bool isWithTimeout);
  bool DoTryLock();
  void DoUnlock();

public:
  GOMutex();
  ~GOMutex();

  std::timed_mutex &GetTimedMutex() { return m_mutex; };

  bool LockOrStop(const char *lockerInfo = NULL, GOThread *pThread = NULL);
  void Lock(const char *lockerInfo = NULL) { LockOrStop(lockerInfo, NULL); }
  void Unlock();
  bool TryLock(const char *lockerInfo = NULL);
  const char *GetLockerInfo() const { return m_LockerInfo; }
};

#endif /* GOMUTEX_H */
