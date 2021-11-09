/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOCONDITION_H
#define GOCONDITION_H

#if defined GO_STD_MUTEX
#include <condition_variable>
#elif defined WX_MUTEX
#include <wx/thread.h>
#else
#include "atomic.h"
#include "GOWaitQueue.h"
#endif

#include "GOMutex.h"
#include "GOrgueThread.h"

class GOCondition
{
public:
  enum {
    SIGNAL_RECEIVED = 0x1,
    MUTEX_LOCKED = 0x2
  };

private:
#if defined GO_STD_MUTEX
  std::timed_mutex &r_mutex;
  std::condition_variable_any m_condition;
#elif defined WX_MUTEX
  wxCondition m_condition;
#else
  atomic_int m_Waiters;
  GOWaitQueue m_Wait;
  GOMutex& m_Mutex;
#endif

  GOCondition(const GOCondition&) = delete;
  const GOCondition& operator=(const GOCondition&) = delete;

  unsigned DoWait(bool isWithTimeout, const char* waiterInfo, GOrgueThread *pThread);
public:
  GOCondition(GOMutex& mutex);
  ~GOCondition();

  /**
   * waits for a signal or for pThread->ShouldStop()
   * Requires that the mutex be acquired before the call
   * After return tries to reaqquire mutex lock
   * @return the bit combination of SIGNAL_RECEIVED and MUTEX_RELOCKED
   */
  unsigned WaitOrStop(const char* waiterInfo = NULL, GOrgueThread* pThread = NULL);
  void Wait() { WaitOrStop(NULL, NULL); }
  void Signal();
  void Broadcast();
};

#endif /* GOCONDITION_H */
