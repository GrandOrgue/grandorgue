/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTouchTask.h"

#include "threading/GOMutexLocker.h"

#include "GOMemoryPool.h"

GOSoundTouchTask::GOSoundTouchTask(GOMemoryPool &pool)
  : GOSoundTaskBase(PRIORITY_TOUCH, false), m_Pool(pool) {}

bool GOSoundTouchTask::DoRun(GOSchedulerThread *pThread) {
  m_Pool.TouchMemory(m_IsToComplete);
  return false;
}

void GOSoundTouchTask::CompleteRound() {
  m_IsToComplete.store(true);

  GOMutexLocker locker(m_mutex);
}
