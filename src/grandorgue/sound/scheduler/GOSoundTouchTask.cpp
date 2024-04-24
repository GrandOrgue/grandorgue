/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTouchTask.h"

#include "GOMemoryPool.h"
#include "threading/GOMutexLocker.h"

GOSoundTouchTask::GOSoundTouchTask(GOMemoryPool &pool)
  : m_Pool(pool), m_Stop(false) {}

unsigned GOSoundTouchTask::GetGroup() { return TOUCH; }

unsigned GOSoundTouchTask::GetCost() { return 0; }

bool GOSoundTouchTask::GetRepeat() { return false; }

void GOSoundTouchTask::Run(GOSoundThread *thread) {
  GOMutexLocker locker(m_Mutex);
  m_Pool.TouchMemory(m_Stop);
}

void GOSoundTouchTask::Exec() {
  m_Stop = true;
  GOMutexLocker locker(m_Mutex);
}

void GOSoundTouchTask::Clear() { Reset(); }

void GOSoundTouchTask::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Stop = false;
}
