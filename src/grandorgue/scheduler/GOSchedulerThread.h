/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSCHEDULERTHREAD_H
#define GOSCHEDULERTHREAD_H

#include <atomic>

#include "threading/GOThread.h"

class GOScheduler;

class GOSchedulerThread : public GOThread {
private:
  GOScheduler *m_Scheduler;

  /* A wakeup that has been requested but not yet consumed. Deliberately
     lock-free: Wakeup() runs in the audio callback path. atomic::wait(false)
     re-reads the current value at the moment it is called, so a store that
     already landed is observed immediately - closing the race that used to
     let Wakeup()/Entry() lose wakeups. The remaining gap - a stop request
     with no matching wakeup - is closed by MarkForStop() itself always
     waking the thread, see below */
  std::atomic_bool m_IsWakeupPending{false};
  /* Set once the worker has drained the scheduler and is about to park;
     cleared once it stops parking to look for more work. WaitForIdle() is
     only ever called from a single thread with no concurrent callers on the
     same instance */
  std::atomic_bool m_IsIdle{false};

  void Entry();

public:
  GOSchedulerThread(GOScheduler *scheduler);
  /** Calls Delete() before base class destruction: ~GOThread()'s fallback
   * Stop() calls MarkForStop() through a GOThread-typed this (the vtable is
   * already unwound to GOThread by then), so the MarkForStop() override
   * below is unreachable from there - only an explicit destructor, running
   * while this object is still fully a GOSchedulerThread, can guarantee the
   * thread is woken and joined before teardown */
  ~GOSchedulerThread() { Delete(); }

  /**
   * === Prerequisites ===
   * During the execution the following must be true:
   * 1. m_Scheduler->GetNextTask() always returns nullptr
   * 2. thread is running and is not marked to be stopped
   *
   * === Result ===
   * Method returns when the thread is idle (i.e. does not run any work items)
   * (so that work items can be deleted safely)
   */
  void WaitForIdle();
  void Run() { Start(); }
  void Delete();
  void Wakeup();

  /** Also wakes the thread, so a stop request is never missed regardless of
   * caller - including GOThread::~GOThread()'s fallback Stop(), which calls
   * this but has no Wakeup() call of its own */
  void MarkForStop() override;
};

#endif
