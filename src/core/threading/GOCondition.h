/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCONDITION_H
#define GOCONDITION_H

#include <condition_variable>

#include "GOMutex.h"
#include "GOThread.h"

class GOCondition {
public:
  enum { SIGNAL_RECEIVED = 0x1, MUTEX_LOCKED = 0x2 };

private:
  std::timed_mutex &r_mutex;
  std::condition_variable_any m_condition;

  GOCondition(const GOCondition &) = delete;
  const GOCondition &operator=(const GOCondition &) = delete;

  unsigned DoWait(
    bool isWithTimeout, const char *waiterInfo, GOThread *pThread);

public:
  GOCondition(GOMutex &mutex);
  ~GOCondition();

  /**
   * waits for a signal or for pThread->ShouldStop()
   * Requires that the mutex be acquired before the call
   * After return tries to reaqquire mutex lock
   * @return the bit combination of SIGNAL_RECEIVED and MUTEX_RELOCKED
   */
  unsigned WaitOrStop(const char *waiterInfo = NULL, GOThread *pThread = NULL);
  void Wait() { WaitOrStop(NULL, NULL); }

  /**
   * Waits for a signal for at most THREADING_WAIT_TIMEOUT and returns.
   * Requires the mutex to be held; it is released while waiting and
   * reacquired before returning. Unlike WaitOrStop() this returns on timeout
   * as well, so the caller must re-check its own predicate in a loop - which
   * is what makes it safe against a notify that was issued before the waiter
   * enqueued on the condition.
   * @return the bit combination of SIGNAL_RECEIVED and MUTEX_LOCKED
   */
  unsigned WaitWithTimeout(const char *waiterInfo = NULL) {
    return DoWait(true, waiterInfo, NULL);
  }

  void Signal();
  void Broadcast();
};

#endif /* GOCONDITION_H */
