/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTOUCHTASK_H
#define GOSOUNDTOUCHTASK_H

#include "GOSoundTaskBase.h"

class GOMemoryPool;
class GOSchedulerThread;

class GOSoundTouchTask : public GOSoundTaskBase {
private:
  GOMemoryPool &m_Pool;

  bool DoRun(GOSchedulerThread *pThread) override;

public:
  GOSoundTouchTask(GOMemoryPool &pool);

  void CompleteRound() override;
};

#endif
