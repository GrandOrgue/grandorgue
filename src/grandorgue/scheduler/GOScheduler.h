/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSCHEDULER_H
#define GOSCHEDULER_H

#include <atomic>
#include <vector>

#include "threading/GOMutex.h"

class GOSchedulerTask;

class GOScheduler {
private:
  std::vector<GOSchedulerTask *> m_Work;
  std::vector<GOSchedulerTask **> m_Tasks;
  // if GetNextTask() always returns nullptr
  std::atomic_bool m_IsNotGivingWork;
  std::atomic_uint m_NextItem;
  std::atomic_uint m_ItemCount;
  unsigned m_RepeatCount;
  GOMutex m_Mutex;

  void Lock() { m_ItemCount.store(0); }
  void Unlock() { m_ItemCount.store(m_Tasks.size()); }
  void Update();

  bool CompareItem(GOSchedulerTask *a, GOSchedulerTask *b);
  void SortList(std::vector<GOSchedulerTask *> &list);
  void NewRoundList(std::vector<GOSchedulerTask *> &list);
  void AddList(GOSchedulerTask *item, std::vector<GOSchedulerTask *> &list);
  void RemoveList(GOSchedulerTask *item, std::vector<GOSchedulerTask *> &list);
  void CompleteRoundList(std::vector<GOSchedulerTask *> &list);

public:
  GOScheduler();
  ~GOScheduler();

  void SetRepeatCount(unsigned count);

  void Clear();
  void NewRound();
  void CompleteRound();
  void Add(GOSchedulerTask *item);
  void Remove(GOSchedulerTask *item);

  /**
   * Checks if any tasks have been added to the scheduler and have not yet been
   * removed.
   * @return true if there are no tasks, false if there are some tasks
   */
  bool IsEmpty() const {
    /* m_Tasks is always rebuilt from m_Work via Update(), so m_Work.empty()
     * implies m_Tasks.empty(). Clear() calls Update(), so after Clear() both
     * are guaranteed empty */
    return m_Work.empty();
  }

  void PauseGivingWork() { m_IsNotGivingWork.store(true); }
  void ResumeGivingWork() { m_IsNotGivingWork.store(false); }

  GOSchedulerTask *GetNextTask();
};

#endif
