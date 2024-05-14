/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSOUNDSCHEDULER_H
#define GOSOUNDSCHEDULER_H

#include <vector>

#include "threading/GOMutex.h"

class GOSoundTask;

class GOSoundScheduler {
private:
  std::vector<GOSoundTask *> m_Work;
  std::vector<GOSoundTask **> m_Tasks;
  // if GetNextGroup() always returns nullptr
  std::atomic_bool m_IsNotGivingWork;
  std::atomic_uint m_NextItem;
  std::atomic_uint m_ItemCount;
  unsigned m_RepeatCount;
  GOMutex m_Mutex;

  void Lock() { m_ItemCount.store(0); }
  void Unlock() { m_ItemCount.store(m_Tasks.size()); }
  void Update();

  bool CompareItem(GOSoundTask *a, GOSoundTask *b);
  void SortList(std::vector<GOSoundTask *> &list);
  void ResetList(std::vector<GOSoundTask *> &list);
  void AddList(GOSoundTask *item, std::vector<GOSoundTask *> &list);
  void RemoveList(GOSoundTask *item, std::vector<GOSoundTask *> &list);
  void ExecList(std::vector<GOSoundTask *> &list);

public:
  GOSoundScheduler();
  ~GOSoundScheduler();

  void SetRepeatCount(unsigned count);

  void Clear();
  void Reset();
  void Exec();
  void Add(GOSoundTask *item);
  void Remove(GOSoundTask *item);

  void PauseGivingWork() { m_IsNotGivingWork.store(true); }
  void ResumeGivingWork() { m_IsNotGivingWork.store(false); }

  GOSoundTask *GetNextGroup();
};

#endif
