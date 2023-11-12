/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSCHEDULER_H
#define GOSOUNDSCHEDULER_H

#include <vector>

#include "threading/GOMutex.h"

class GOSoundWorkItem;

class GOSoundScheduler {
private:
  std::vector<GOSoundWorkItem *> m_Work;
  std::vector<GOSoundWorkItem **> m_WorkItems;
  // if GetNextGroup() always returns nullptr
  std::atomic_bool m_IsNotGivingWork;
  std::atomic_uint m_NextItem;
  std::atomic_uint m_ItemCount;
  unsigned m_RepeatCount;
  GOMutex m_Mutex;

  void Lock() { m_ItemCount.store(0); }
  void Unlock() { m_ItemCount.store(m_WorkItems.size()); }
  void Update();

  bool CompareItem(GOSoundWorkItem *a, GOSoundWorkItem *b);
  void SortList(std::vector<GOSoundWorkItem *> &list);
  void ResetList(std::vector<GOSoundWorkItem *> &list);
  void AddList(GOSoundWorkItem *item, std::vector<GOSoundWorkItem *> &list);
  void RemoveList(GOSoundWorkItem *item, std::vector<GOSoundWorkItem *> &list);
  void ExecList(std::vector<GOSoundWorkItem *> &list);

public:
  GOSoundScheduler();
  ~GOSoundScheduler();

  void SetRepeatCount(unsigned count);

  void Clear();
  void Reset();
  void Exec();
  void Add(GOSoundWorkItem *item);
  void Remove(GOSoundWorkItem *item);

  void PauseGivingWork() { m_IsNotGivingWork.store(true); }
  void ResumeGivingWork() { m_IsNotGivingWork.store(false); }

  GOSoundWorkItem *GetNextGroup();
};

#endif
