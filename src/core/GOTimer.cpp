/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTimer.h"

#include "GOTimerCallback.h"
#include "threading/GOMutexLocker.h"

GOTimer::GOTimer() : wxTimer(), m_Entries() {}

GOTimer::~GOTimer() { wxTimer::Stop(); }

void GOTimer::SetRelativeTimer(
  GOTime time, GOTimerCallback *callback, unsigned interval) {
  time += wxGetLocalTimeMillis();
  SetTimer(time, callback, interval);
}

void GOTimer::SetTimer(
  GOTime time, GOTimerCallback *callback, unsigned interval) {
  GOTimerEntry e;
  e.time = time;
  e.callback = callback;
  e.interval = interval;
  bool added = false;
  for (unsigned i = 0; !added && i < m_Entries.size(); i++)
    if (m_Entries[i].callback == NULL) {
      m_Entries[i] = e;
      added = true;
    }
  if (!added)
    m_Entries.push_back(e);

  GOMutexLocker locker(m_Lock, true);
  if (locker.IsLocked())
    Schedule();
}

void GOTimer::UpdateInterval(GOTimerCallback *callback, unsigned interval) {
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].callback == callback)
      m_Entries[i].interval = interval;
}

void GOTimer::DeleteTimer(GOTimerCallback *callback) {
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].callback == callback)
      m_Entries[i].callback = NULL;
}

void GOTimer::Schedule() {
  GOTime next = -1;
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].callback) {
      if (next == -1 || next > m_Entries[i].time)
        next = m_Entries[i].time;
    }
  if (next != -1) {
    GOTime now = wxGetLocalTimeMillis();
    GOTime wait;
    if (next > now)
      wait = next - now;
    else
      wait = 1;
    wxTimer::Start(wait.GetValue(), true);
  }
}

void GOTimer::Notify() {
  GOMutexLocker locker(m_Lock);

  GOTime now = wxGetLocalTimeMillis();
  for (unsigned i = 0; i < m_Entries.size(); i++)
    if (m_Entries[i].callback && m_Entries[i].time <= now) {
      m_Entries[i].callback->HandleTimer();
      if (m_Entries[i].interval) {
        if (m_Entries[i].time <= now + m_Entries[i].interval)
          m_Entries[i].time = now;
        m_Entries[i].time += m_Entries[i].interval;
      } else
        m_Entries[i].callback = NULL;
    }
  Schedule();
}
