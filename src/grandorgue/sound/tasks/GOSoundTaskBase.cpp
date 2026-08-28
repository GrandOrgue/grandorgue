/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTaskBase.h"

#include "scheduler/GOSchedulerThread.h"
#include "threading/GOMutexLocker.h"

GOSoundTaskBase::GOSoundTaskBase(TaskPriority priority, bool isRepeatable)
  : m_priority(priority),
    m_IsRepeatable(isRepeatable),
    m_IsToComplete(false),
    m_RunState(RUN_STATE_NOT_STARTED) {}

// Only moves m_RunState between RUN_STATE_NOT_STARTED and RUN_STATE_DONE:
// DoRun() runs entirely under m_mutex, so a cooperative subclass that needs
// several threads inside DoRun() at once (e.g. GOSoundGroupTask) cannot fit
// this default protocol and overrides Run() itself instead.
void GOSoundTaskBase::Run(GOSchedulerThread *pThread) {
  if (m_RunState.load() < RUN_STATE_DONE) {
    GOMutexLocker locker(m_mutex, false, "GOSoundTaskBase::Run", pThread);

    if (
      locker.IsLocked() && m_RunState.load() < RUN_STATE_DONE && DoRun(pThread))
      m_RunState.store(RUN_STATE_DONE);
  }
}

void GOSoundTaskBase::CompleteRound() {
  m_IsToComplete.store(true);
  Run();
}

void GOSoundTaskBase::NewRound() {
  GOMutexLocker locker(m_mutex);

  m_IsToComplete.store(false);
  m_RunState.store(RUN_STATE_NOT_STARTED);
  DoNewRound();
}
