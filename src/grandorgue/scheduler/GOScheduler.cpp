/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOScheduler.h"

#include <cassert>

#include "scheduler/GOSchedulerTask.h"
#include "threading/GOMutexLocker.h"

GOScheduler::GOScheduler()
  : m_Work(),
    m_Tasks(),
    m_IsNotGivingWork(false),
    m_ItemCount(0),
    m_RepeatCount(0) {}

GOScheduler::~GOScheduler() {
  GOMutexLocker lock(m_Mutex);
  Lock();
}

void GOScheduler::SetRepeatCount(unsigned count) {
  m_RepeatCount = count;
  GOMutexLocker lock(m_Mutex);
  Lock();
  Update();
  Unlock();
}

void GOScheduler::Clear() {
  GOMutexLocker lock(m_Mutex);
  Lock();
  for (GOSchedulerTask *item : m_Work)
    if (item)
      item->DiscardContent();
  m_Work.clear();
  Update();
  Unlock();
}

void GOScheduler::Update() {
  m_Tasks.clear();
  SortList(m_Work);
  for (unsigned i = 0; i < m_Work.size();) {
    unsigned cnt = 1;
    while (i + cnt < m_Work.size()
           && m_Work[i]->GetPriority() == m_Work[i + cnt]->GetPriority())
      cnt++;
    unsigned rcnt = m_Work[i]->IsRepeatable() ? m_RepeatCount : 1;
    for (unsigned j = 0; j < rcnt; j++)
      for (unsigned k = 0; k < cnt; k++)
        m_Tasks.push_back(&m_Work[i + k]);
    i += cnt;
  }
}

void GOScheduler::AddList(
  GOSchedulerTask *item, std::vector<GOSchedulerTask *> &list) {
  list.push_back(item);
}

void GOScheduler::Add(GOSchedulerTask *item) {
  if (!item)
    return;
  assert(item->IsEmpty());
  GOMutexLocker lock(m_Mutex);
  Lock();
  AddList(item, m_Work);
  Update();
  Unlock();
}

void GOScheduler::RemoveList(
  GOSchedulerTask *item, std::vector<GOSchedulerTask *> &list) {
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i] == item)
      list[i] = nullptr;
}

void GOScheduler::Remove(GOSchedulerTask *item) {
  GOMutexLocker lock(m_Mutex);
  RemoveList(item, m_Work);
}

bool GOScheduler::CompareItem(GOSchedulerTask *a, GOSchedulerTask *b) {
  if (a && b) {
    if (a->GetPriority() > b->GetPriority())
      return true;
    if (a->GetCost() < b->GetCost())
      return true;
    return false;
  }
  if (!a && b)
    return true;
  return false;
}

void GOScheduler::SortList(std::vector<GOSchedulerTask *> &list) {
  for (unsigned i = 0; i + 1 < list.size(); i++) {
    for (unsigned j = i; j > 0 && CompareItem(list[j], list[j + 1]); j--) {
      GOSchedulerTask *tmp = list[j];
      list[j] = list[j + 1];
      list[j + 1] = tmp;
    }
  }
}

void GOScheduler::NewRoundList(std::vector<GOSchedulerTask *> &list) {
  SortList(list);
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i])
      list[i]->NewRound();
}

void GOScheduler::NewRound() {
  GOMutexLocker lock(m_Mutex);
  NewRoundList(m_Work);
  m_NextItem.exchange(0);
}

void GOScheduler::CompleteRoundList(std::vector<GOSchedulerTask *> &list) {
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i])
      list[i]->CompleteRound();
}

void GOScheduler::CompleteRound() {
  GOMutexLocker lock(m_Mutex);
  CompleteRoundList(m_Work);
}

GOSchedulerTask *GOScheduler::GetNextTask() {
  do {
    if (m_IsNotGivingWork.load()) {
      return nullptr;
    }
    unsigned next = m_NextItem.fetch_add(1);
    if (next >= m_ItemCount)
      return nullptr;
    GOSchedulerTask *item = *m_Tasks[next];
    if (item)
      return item;
  } while (true);
}
