/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTHREAD_H
#define GOSOUNDTHREAD_H

#include "threading/GOCondition.h"
#include "threading/GOMutex.h"
#include "threading/GOThread.h"

class GOSoundScheduler;

class GOSoundThread : public GOThread {
private:
  GOSoundScheduler *m_Scheduler;

  GOMutex m_Mutex;
  GOCondition m_Condition;
  GOCondition m_IdleStateReachedCondition;
  // whether the thread sleeps and waits for waking up with m_Condition
  bool m_IsIdle; // guarded by m_Mutex

  void Entry();

public:
  GOSoundThread(GOSoundScheduler *scheduler);

  /*
   * === Prerequisites ===
   * During the execution the following must be true:
   * 1. m_Scheduler->GetNextGroup() always returns nullptr
   * 2. thread is running and is not marked to be stopped
   *
   * === Result ===
   * Method returns when the thread is idle (i.e. does not run any work items)
   * (so that work items can be deleted safely)
   */
  void WaitForIdle();
  void Run();
  void Delete();
  void Wakeup();
};

#endif
