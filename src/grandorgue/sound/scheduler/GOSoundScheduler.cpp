/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSoundScheduler.h"

#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOMutexLocker.h"

GOSoundScheduler::GOSoundScheduler()
  : m_Work(),
    m_WorkItems(),
    m_IsNotGivingWork(false),
    m_ItemCount(0),
    m_RepeatCount(0) {}

GOSoundScheduler::~GOSoundScheduler() {
  GOMutexLocker lock(m_Mutex);
  Lock();
}

void GOSoundScheduler::SetRepeatCount(unsigned count) {
  m_RepeatCount = count;
  GOMutexLocker lock(m_Mutex);
  Lock();
  Update();
  Unlock();
}

void GOSoundScheduler::Clear() {
  GOMutexLocker lock(m_Mutex);
  Lock();
  m_Work.clear();
  Update();
  Unlock();
}

void GOSoundScheduler::Update() {
  m_WorkItems.clear();
  SortList(m_Work);
  for (unsigned i = 0; i < m_Work.size();) {
    unsigned cnt = 1;
    while (i + cnt < m_Work.size()
           && m_Work[i]->GetGroup() == m_Work[i + cnt]->GetGroup())
      cnt++;
    unsigned rcnt = m_Work[i]->GetRepeat() ? m_RepeatCount : 1;
    for (unsigned j = 0; j < rcnt; j++)
      for (unsigned k = 0; k < cnt; k++)
        m_WorkItems.push_back(&m_Work[i + k]);
    i += cnt;
  }
}

void GOSoundScheduler::AddList(
  GOSoundWorkItem *item, std::vector<GOSoundWorkItem *> &list) {
  list.push_back(item);
}

void GOSoundScheduler::Add(GOSoundWorkItem *item) {
  if (!item)
    return;
  item->Clear();
  GOMutexLocker lock(m_Mutex);
  Lock();
  AddList(item, m_Work);
  Update();
  Unlock();
}

void GOSoundScheduler::RemoveList(
  GOSoundWorkItem *item, std::vector<GOSoundWorkItem *> &list) {
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i] == item)
      list[i] = nullptr;
}

void GOSoundScheduler::Remove(GOSoundWorkItem *item) {
  GOMutexLocker lock(m_Mutex);
  RemoveList(item, m_Work);
}

bool GOSoundScheduler::CompareItem(GOSoundWorkItem *a, GOSoundWorkItem *b) {
  if (a && b) {
    if (a->GetGroup() > b->GetGroup())
      return true;
    if (a->GetCost() < b->GetCost())
      return true;
    return false;
  }
  if (!a && b)
    return true;
  return false;
}

void GOSoundScheduler::SortList(std::vector<GOSoundWorkItem *> &list) {
  for (unsigned i = 0; i + 1 < list.size(); i++) {
    for (unsigned j = i; j > 0 && CompareItem(list[j], list[j + 1]); j--) {
      GOSoundWorkItem *tmp = list[j];
      list[j] = list[j + 1];
      list[j + 1] = tmp;
    }
  }
}

void GOSoundScheduler::ResetList(std::vector<GOSoundWorkItem *> &list) {
  SortList(list);
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i])
      list[i]->Reset();
}

void GOSoundScheduler::Reset() {
  GOMutexLocker lock(m_Mutex);
  ResetList(m_Work);
  m_NextItem.exchange(0);
}

void GOSoundScheduler::ExecList(std::vector<GOSoundWorkItem *> &list) {
  for (unsigned i = 0; i < list.size(); i++)
    if (list[i])
      list[i]->Exec();
}

void GOSoundScheduler::Exec() {
  GOMutexLocker lock(m_Mutex);
  ExecList(m_Work);
}

GOSoundWorkItem *GOSoundScheduler::GetNextGroup() {
  do {
    if (m_IsNotGivingWork.load()) {
      return nullptr;
    }
    unsigned next = m_NextItem.fetch_add(1);
    if (next >= m_ItemCount)
      return nullptr;
    GOSoundWorkItem *item = *m_WorkItems[next];
    if (item)
      return item;
  } while (true);
}
