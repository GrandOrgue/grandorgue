/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSchedulerThread.h"

#include <unistd.h>

#include <wx/log.h>

#include "scheduler/GOSchedulerTask.h"
#include "threading/GOMutexLocker.h"

#include "GOScheduler.h"

GOSchedulerThread::GOSchedulerThread(GOScheduler *scheduler)
  : GOThread(),
    m_Scheduler(scheduler),
    m_Condition(m_Mutex),
    m_IdleStateReachedCondition(m_Mutex),
    m_IsIdle(false) {
  wxLogDebug(wxT("Create Thread"));
}

void GOSchedulerThread::Entry() {
  while (!ShouldStop()) {
    bool shouldStop = false;

    do {
      GOSchedulerTask *next = m_Scheduler->GetNextTask();

      if (next == NULL)
        break;
      next->Run(this);
      shouldStop = ShouldStop();
    } while (!shouldStop);

    if (shouldStop)
      break;

    GOMutexLocker lock(m_Mutex, false, "GOSchedulerThread::Entry", this);
    if (!lock.IsLocked() || ShouldStop())
      break;
    m_IsIdle = true;
    m_IdleStateReachedCondition.Broadcast();
    if (!m_Condition.WaitOrStop("GOSchedulerThread::Entry"))
      break;
    m_IsIdle = false;
  }

  return;
}

void GOSchedulerThread::WaitForIdle() {
  GOMutexLocker lock(m_Mutex, false, "GOSchedulerThread::WaitForIdle");
  while (!m_IsIdle) {
    m_IdleStateReachedCondition.Wait();
  }
}

void GOSchedulerThread::Run() { Start(); }

void GOSchedulerThread::Wakeup() { m_Condition.Signal(); }

void GOSchedulerThread::Delete() {
  MarkForStop();
  Wakeup();
  Wait();
}
