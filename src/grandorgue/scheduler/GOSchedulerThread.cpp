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

#include "GOScheduler.h"

GOSchedulerThread::GOSchedulerThread(GOScheduler *scheduler)
  : GOThread(), m_Scheduler(scheduler) {
  wxLogDebug(wxT("Create Thread"));
}

void GOSchedulerThread::Entry() {
  while (true) {
    bool shouldStop;

    do {
      shouldStop = ShouldStop();
      if (shouldStop)
        break;

      GOSchedulerTask *next = m_Scheduler->GetNextTask();

      if (next == NULL)
        break;
      next->Run(this);
    } while (true);

    if (shouldStop)
      break;

    // Toggled true/false once per pass, not just once before this loop
    // exits for good: WaitForIdle() is called repeatedly over the thread's
    // whole lifetime (e.g. once per StopEngine()), each time expecting to
    // observe idle for that pass, then the thread keeps running afterwards
    // (StartEngine() resumes it) - so "idle" must be reported and cleared on
    // every parking cycle, not only at final shutdown.
    m_IsIdle.store(true);
    m_IsIdle.notify_all();

    while (!m_IsWakeupPending.exchange(false) && !ShouldStop())
      m_IsWakeupPending.wait(false);

    m_IsIdle.store(false);
  }
}

void GOSchedulerThread::WaitForIdle() {
  while (!m_IsIdle.load())
    m_IsIdle.wait(false);
}

void GOSchedulerThread::Wakeup() {
  m_IsWakeupPending.store(true);
  m_IsWakeupPending.notify_one();
}

void GOSchedulerThread::MarkForStop() {
  GOThread::MarkForStop();
  Wakeup();
}

void GOSchedulerThread::Delete() {
  MarkForStop();
  Wait();
}
