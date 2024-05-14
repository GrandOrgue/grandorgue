/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTOUCHTASK_H
#define GOSOUNDTOUCHTASK_H

#include <atomic>

#include "GOSoundTask.h"
#include "threading/GOMutex.h"

class GOMemoryPool;

class GOSoundTouchTask : public GOSoundTask {
private:
  GOMemoryPool &m_Pool;
  GOMutex m_Mutex;
  std::atomic_bool m_Stop;

public:
  GOSoundTouchTask(GOMemoryPool &pool);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();
};

#endif
