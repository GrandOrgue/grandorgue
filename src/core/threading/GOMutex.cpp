/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMutex.h"

#include "threading_impl.h"

#define GO_PRINTCONTENTION 0

static const char *const UNKNOWN_LOCKER_INFO = "UnknownLocker";

#define LOCKER_INFO(lockerInfo) (lockerInfo ? lockerInfo : UNKNOWN_LOCKER_INFO)

GOMutex::GOMutex()
  :
#if !defined GO_STD_MUTEX && !defined WX_MUTEX
    m_Lock(0),
#endif
    m_LockerInfo(NULL) {
}

GOMutex::~GOMutex() {}

#if defined GO_STD_MUTEX

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

#elif defined WX_MUTEX

bool DoLock(bool isWithTimeout) {
  bool isLocked;

  if (isWithTimeout)
    isLocked = m_Mutex.LockTimeout(WAIT_TIMEOUT_MS) != wxMUTEX_TIMEOUT;
  else {
    m_Mutex.Lock();
    isLocked = true;
  }
  return isLocked;
}

void GOMutex::DoUnlock() { m_Mutex.Unlock(); }

bool GOMutex::DoTryLock() { return m_Mutex.TryLock() == wxMUTEX_NO_ERROR; }

#else

bool GOMutex::DoLock(bool isWithTimeout) {
  bool isLocked;

  int value = m_Lock.fetch_add(1);

  if (!value) {
    __sync_synchronize();
    isLocked = true;
  } else {
#if GO_PRINTCONTENTION
    wxLogWarning(wxT("Mutex::wait %p"), this);
    GOStackPrinter::printStack(this);
#endif
    isLocked = m_Wait.Wait(isWithTimeout);
#if GO_PRINTCONTENTION
    wxLogWarning(
      wxT("Mutex::end_wait %p", isLocked = % s),
      this,
      isLocked ? "true" : "false");
#endif
  }
  return isLocked;
}

void GOMutex::DoUnlock() {
  __sync_synchronize();

  int value = m_Lock.fetch_add(-1);

  if (value > 1)
    m_Wait.Wakeup();
}

bool GOMutex::DoTryLock() {
  int old = 0;

  if (m_Lock.compare_exchange(old, 1)) {
    __sync_synchronize();
    return true;
  }
  return false;
}

#endif

bool GOMutex::LockOrStop(const char *lockerInfo, GOThread *pThread) {
  bool isLocked = false;

  if (pThread != NULL) {
    bool isFirstTime = true;

    while (!pThread->ShouldStop() && !isLocked) {
      isLocked = DoLock(true);
      if (!isLocked && isFirstTime) {
        const char *currentLockerInfo = m_LockerInfo;

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
    m_LockerInfo = LOCKER_INFO(lockerInfo);
  return isLocked;
}

void GOMutex::Unlock() {
  m_LockerInfo = NULL;
  DoUnlock();
}

bool GOMutex::TryLock(const char *lockerInfo) {
  bool isLocked = DoTryLock();

  if (isLocked)
    m_LockerInfo = LOCKER_INFO(lockerInfo);
  return isLocked;
}
