/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMutexLocker.h"

GOMutexLocker::GOMutexLocker(
  GOMutex &mutex, bool try_lock, const char *lockerInfo, GOThread *pThread)
  : m_mutex(mutex), m_IsLocked(false) {
  if (try_lock)
    m_IsLocked = m_mutex.TryLock(lockerInfo);
  else if (pThread != NULL)
    m_IsLocked = m_mutex.LockOrStop(lockerInfo, pThread);
  else {
    m_mutex.Lock(lockerInfo);
    m_IsLocked = true;
  }
}

GOMutexLocker::~GOMutexLocker() {
  if (m_IsLocked)
    m_mutex.Unlock();
}

bool GOMutexLocker::TryLock(const char *lockerInfo) {
  if (!m_IsLocked)
    m_IsLocked = m_mutex.TryLock(lockerInfo);
  return m_IsLocked;
}

void GOMutexLocker::Unlock() {
  if (m_IsLocked)
    m_mutex.Unlock();
  m_IsLocked = false;
}
