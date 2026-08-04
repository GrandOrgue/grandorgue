/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTASKBASE_H
#define GOSOUNDTASKBASE_H

#include "scheduler/GOSchedulerTask.h"

/** A GOSchedulerTask that belongs to the sound engine */
class GOSoundTaskBase : public GOSchedulerTask {
public:
  /**
   * Scheduling priority of a sound task: a task with a smaller value is
   * scheduled earlier. Tasks with the same value form a batch that the
   * scheduler emits together. The order is an optimisation only: a task
   * lazily runs its prerequisites itself (see EnsureBufferReady,
   * GOSoundWindchestTask::GetVolume).
   */
  enum TaskPriority {
    PRIORITY_TREMULANT = 10,
    PRIORITY_WINDCHEST = 20,
    PRIORITY_AUDIOGROUP = 50,
    PRIORITY_AUDIOOUTPUT = 100,
    PRIORITY_AUDIORECORDER = 150,
    PRIORITY_RELEASE = 160,
    PRIORITY_TOUCH = 700,
  };
};

#endif /* GOSOUNDTASKBASE_H */
