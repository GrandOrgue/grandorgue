/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
