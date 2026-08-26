/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTOUCHTASK_H
#define GOSOUNDTOUCHTASK_H

#include <atomic>

#include "threading/GOMutex.h"

#include "GOSoundTaskBase.h"

class GOMemoryPool;
class GOSchedulerThread;

class GOSoundTouchTask : public GOSoundTaskBase {
private:
  GOMemoryPool &m_Pool;
  GOMutex m_Mutex;
  std::atomic_bool m_IsToComplete;

public:
  GOSoundTouchTask(GOMemoryPool &pool);

  unsigned GetPriority() const override { return PRIORITY_TOUCH; }
  unsigned GetCost() const override { return 0; }
  bool IsRepeatable() const override { return false; }
  void Run(GOSchedulerThread *pThread = nullptr) override;
  void CompleteRound() override;

  void DiscardContent() override { NewRound(); }
  void NewRound() override;
};

#endif
