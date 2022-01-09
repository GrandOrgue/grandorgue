/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCondition.h"

#include "threading_impl.h"

const char *const UNKNOWN_WAITER_INFO = "UnknownWaiter";

#define WAITER_INFO(waiterInfo) (waiterInfo ? waiterInfo : UNKNOWN_WAITER_INFO)

/**
 * Composes the result of the wait for condition.
 * @param isSignalReceived
 * @param isMutexLocked
 * @return the bitwise composit result from GOCondition::SIGNAL_RECEIVED and
 * GOCondition::MUTEX_LOCKED
 */

unsigned compose_result(bool isSignalReceived, bool isMutexLocked) {
  return (isSignalReceived ? GOCondition::SIGNAL_RECEIVED : 0)
    | (isMutexLocked ? GOCondition::MUTEX_LOCKED : 0);
}

#if defined GO_STD_MUTEX
GOCondition::GOCondition(GOMutex &mutex) : r_mutex(mutex.GetTimedMutex()) {}

GOCondition::~GOCondition() {}

unsigned GOCondition::DoWait(
  bool isWithTimeout, const char *waiterInfo, GOThread *) {
  bool isSignalReceived;

  if (isWithTimeout)
    isSignalReceived = m_condition.wait_for(r_mutex, THREADING_WAIT_TIMEOUT)
      != std::cv_status::timeout;
  else {
    m_condition.wait(r_mutex);
    isSignalReceived = true;
  }
  return compose_result(isSignalReceived, true);
}

void GOCondition::Signal() { m_condition.notify_one(); }

void GOCondition::Broadcast() { m_condition.notify_all(); }
#elif defined WX_MUTEX

GOCondition::GOCondition(GOMutex &mutex) : m_condition(mutex.m_Mutex) {}

GOCondition::~GOCondition() {}

unsigned GOCondition::DoWait(
  bool isWithTimeout, const char *waiterInfo, GOThread *) {
  bool isSignalReceived;

  if (isWithTimeout)
    isSignalReceived
      = m_condition.WaitTimeout(WAIT_TIMEOUT_MS) != wxCOND_TIMEOUT;
  else {
    m_condition.Wait();
    rc = true;
  }
  return compose_result(isSignalReceived, true);
}

void GOCondition::Signal() { m_condition.Signal(); }

void GOCondition::Broadcast() { m_condition.Broadcast(); }

#else

GOCondition::GOCondition(GOMutex &mutex) : m_Waiters(0), m_Mutex(mutex) {}

GOCondition::~GOCondition() {
  while (m_Waiters > 0)
    Signal();
}

unsigned GOCondition::DoWait(
  bool isWithTimeout, const char *waiterInfo, GOThread *pThread) {
  m_Waiters.fetch_add(1);
  m_Mutex.Unlock();

  bool isSignalReceived = m_Wait.Wait(isWithTimeout);

  // now try to lock the mutex again
  bool isMutexLocked = false;
  bool isFirstTime = true;

  do {
    isMutexLocked = m_Mutex.LockOrStop(waiterInfo, pThread);
    if (isMutexLocked)
      break;

    // a timeout occured
    if (isFirstTime && waiterInfo) {
      wxLogWarning(
        "GOCondition::Wait: unable to restore lock on the condition mutex "
        "%p:%p for %s",
        this,
        &m_Mutex,
        wxString(WAITER_INFO(waiterInfo)));
      isFirstTime = false;
    }
  } while (pThread == NULL || !pThread->ShouldStop());
  return compose_result(isSignalReceived, isMutexLocked);
}

void GOCondition::Signal() {
  int waiters = m_Waiters.fetch_add(-1);
  if (waiters <= 0)
    m_Waiters.fetch_add(+1);
  else
    m_Wait.Wakeup();
}

void GOCondition::Broadcast() {
  int waiters;
  do {
    waiters = m_Waiters.fetch_add(-1);
    if (waiters <= 0) {
      m_Waiters.fetch_add(+1);
      return;
    } else
      m_Wait.Wakeup();
  } while (waiters > 1);
}

#endif

unsigned GOCondition::WaitOrStop(const char *waiterInfo, GOThread *pThread) {
  unsigned rc = 0;
  bool isFirstTime = true;

  while (pThread == NULL || !pThread->ShouldStop()) {
    rc = DoWait(pThread != NULL, waiterInfo, pThread);

    if (rc & SIGNAL_RECEIVED)
      break;

    // timeout occured
    if (isFirstTime && waiterInfo) {
      wxLogWarning(
        "GOCondition::WaitOrStop: timeout while %s waited for condition %p",
        wxString(WAITER_INFO(waiterInfo)),
        this);
      isFirstTime = false;
    }
  }
  return rc;
}
