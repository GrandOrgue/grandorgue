/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundTouchWorkItem.h"

#include "GOMemoryPool.h"
#include "threading/GOMutexLocker.h"

GOSoundTouchWorkItem::GOSoundTouchWorkItem(GOMemoryPool &pool)
  : m_Pool(pool), m_Stop(false) {}

unsigned GOSoundTouchWorkItem::GetGroup() { return TOUCH; }

unsigned GOSoundTouchWorkItem::GetCost() { return 0; }

bool GOSoundTouchWorkItem::GetRepeat() { return false; }

void GOSoundTouchWorkItem::Run(GOSoundThread *thread) {
  GOMutexLocker locker(m_Mutex);
  m_Pool.TouchMemory(m_Stop);
}

void GOSoundTouchWorkItem::Exec() {
  m_Stop = true;
  GOMutexLocker locker(m_Mutex);
}

void GOSoundTouchWorkItem::Clear() { Reset(); }

void GOSoundTouchWorkItem::Reset() {
  GOMutexLocker locker(m_Mutex);
  m_Stop = false;
}
