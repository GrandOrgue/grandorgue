/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTouchTask.h"

#include "GOMemoryPool.h"
#include "threading/GOMutexLocker.h"

GOSoundTouchTask::GOSoundTouchTask(GOMemoryPool &pool)
  : m_Pool(pool), m_IsToComplete(false) {}

void GOSoundTouchTask::Run(GOSchedulerThread *pThread) {
  GOMutexLocker locker(m_Mutex);
  m_Pool.TouchMemory(m_IsToComplete);
}

void GOSoundTouchTask::CompleteRound() {
  m_IsToComplete = true;
  GOMutexLocker locker(m_Mutex);
}

void GOSoundTouchTask::NewRound() {
  GOMutexLocker locker(m_Mutex);
  m_IsToComplete = false;
}
