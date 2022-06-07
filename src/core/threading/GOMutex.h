/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMUTEX_H
#define GOMUTEX_H

#ifdef GO_STD_MUTEX
#include <mutex>
#elif WX_MUTEX
#include <wx/thread.h>
#else
#include "GOWaitQueue.h"
#include "atomic.h"
#endif

#include "GOThread.h"

class GOMutex {
private:
#if defined GO_STD_MUTEX
  std::timed_mutex m_mutex;
#elif defined WX_MUTEX
  wxMutex m_Mutex;
#else
  GOWaitQueue m_Wait;
  atomic_int m_Lock;
#endif

  const char *volatile m_LockerInfo;

  GOMutex(const GOMutex &) = delete;
  const GOMutex &operator=(const GOMutex &) = delete;

  bool DoLock(bool isWithTimeout);
  bool DoTryLock();
  void DoUnlock();

public:
  GOMutex();
  ~GOMutex();

#if defined GO_STD_MUTEX
  std::timed_mutex &GetTimedMutex() { return m_mutex; };
#endif

  bool LockOrStop(const char *lockerInfo = NULL, GOThread *pThread = NULL);
  void Lock(const char *lockerInfo = NULL) { LockOrStop(lockerInfo, NULL); }
  void Unlock();
  bool TryLock(const char *lockerInfo = NULL);
  const char *GetLockerInfo() const { return m_LockerInfo; }
};

#endif /* GOMUTEX_H */
