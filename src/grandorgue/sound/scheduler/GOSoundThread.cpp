/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundThread.h"

#include <wx/log.h>

#include "GOSoundScheduler.h"
#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOMutexLocker.h"
#include <unistd.h>

GOSoundThread::GOSoundThread(GOSoundScheduler *scheduler)
  : GOThread(),
    m_Scheduler(scheduler),
    m_Condition(m_Mutex),
    m_IdleStateReachedCondition(m_Mutex),
    m_IsIdle(false) {
  wxLogDebug(wxT("Create Thread"));
}

void GOSoundThread::Entry() {
  while (!ShouldStop()) {
    bool shouldStop = false;

    do {
      GOSoundWorkItem *next = m_Scheduler->GetNextGroup();

      if (next == NULL)
        break;
      next->Run(this);
      shouldStop = ShouldStop();
    } while (!shouldStop);

    if (shouldStop)
      break;

    GOMutexLocker lock(m_Mutex, false, "GOSoundThread::Entry", this);
    if (!lock.IsLocked() || ShouldStop())
      break;
    m_IsIdle = true;
    m_IdleStateReachedCondition.Broadcast();
    if (!m_Condition.WaitOrStop("GOSoundThread::Entry"))
      break;
    m_IsIdle = false;
  }

  return;
}

void GOSoundThread::WaitForIdle() {
  GOMutexLocker lock(m_Mutex, false, "GOSoundThread::WaitForIdle");
  while (!m_IsIdle) {
    m_IdleStateReachedCondition.Wait();
  }
}

void GOSoundThread::Run() { Start(); }

void GOSoundThread::Wakeup() { m_Condition.Signal(); }

void GOSoundThread::Delete() {
  MarkForStop();
  Wakeup();
  Wait();
}
