/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDTOUCHWORKITEM_H
#define GOSOUNDTOUCHWORKITEM_H

#include <atomic>

#include "GOSoundWorkItem.h"
#include "threading/GOMutex.h"

class GOMemoryPool;

class GOSoundTouchWorkItem : public GOSoundWorkItem {
private:
  GOMemoryPool &m_Pool;
  GOMutex m_Mutex;
  std::atomic_bool m_Stop;

public:
  GOSoundTouchWorkItem(GOMemoryPool &pool);

  unsigned GetGroup();
  unsigned GetCost();
  bool GetRepeat();
  void Run(GOSoundThread *thread = nullptr);
  void Exec();

  void Clear();
  void Reset();
};

#endif
