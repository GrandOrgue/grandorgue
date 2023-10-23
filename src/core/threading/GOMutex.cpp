/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMutex.h"

#include "threading_impl.h"

#define GO_PRINTCONTENTION 0

static const char *const UNKNOWN_LOCKER_INFO = "UnknownLocker";

#define LOCKER_INFO(lockerInfo) (lockerInfo ? lockerInfo : UNKNOWN_LOCKER_INFO)

GOMutex::GOMutex() : m_LockerInfo(NULL) {}

GOMutex::~GOMutex() {}

bool GOMutex::DoLock(bool isWithTimeout) {
  bool isLocked;

  if (isWithTimeout)
    isLocked = m_mutex.try_lock_for(THREADING_WAIT_TIMEOUT);
  else {
    m_mutex.lock();
    isLocked = true;
  }
  return isLocked;
}

void GOMutex::DoUnlock() { m_mutex.unlock(); }

bool GOMutex::DoTryLock() { return m_mutex.try_lock(); }

bool GOMutex::LockOrStop(const char *lockerInfo, GOThread *pThread) {
  bool isLocked = false;

  if (pThread != NULL) {
    bool isFirstTime = true;

    while (!pThread->ShouldStop() && !isLocked) {
      isLocked = DoLock(true);
      if (!isLocked && isFirstTime) {
        const char *currentLockerInfo = m_LockerInfo.load();

        wxLogWarning(
          "GOMutex: timeout when locking mutex %p; currentLocker=%s "
          "newLocker=%s",
          this,
          wxString(currentLockerInfo),
          wxString(LOCKER_INFO(lockerInfo)));
        isFirstTime = false;
      }
    }
  } else
    isLocked = DoLock(false);

  if (isLocked)
    m_LockerInfo.store(LOCKER_INFO(lockerInfo));
  return isLocked;
}

void GOMutex::Unlock() {
  m_LockerInfo.store(NULL);
  DoUnlock();
}

bool GOMutex::TryLock(const char *lockerInfo) {
  bool isLocked = DoTryLock();

  if (isLocked)
    m_LockerInfo.store(LOCKER_INFO(lockerInfo));
  return isLocked;
}
