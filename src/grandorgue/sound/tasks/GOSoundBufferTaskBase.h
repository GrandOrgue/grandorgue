/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDBUFFERTASKBASE_H
#define GOSOUNDBUFFERTASKBASE_H

#include "sound/buffer/GOSoundBufferManaged.h"

#include "GOSoundTaskBase.h"

class GOSchedulerThread;

class GOSoundBufferTaskBase : public GOSoundTaskBase,
                              public GOSoundBufferManaged {
public:
  GOSoundBufferTaskBase(
    TaskPriority priority,
    bool isRepeatable,
    unsigned nChannels,
    unsigned nFrames)
    : GOSoundTaskBase(priority, isRepeatable),
      GOSoundBufferManaged(nChannels, nFrames) {}

  virtual void EnsureBufferReady(
    bool isToComplete, GOSchedulerThread *pThread = nullptr)
    = 0;
};

#endif /* GOSOUNDBUFFERTASKBASE_H */
