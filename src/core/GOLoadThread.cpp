/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoadThread.h"

#include "GOAlloc.h"
#include "GOCacheObject.h"
#include "GOEventDistributor.h"
#include "GOMemoryPool.h"

GOLoadThread::GOLoadThread(GOEventDistributor& objs, GOMemoryPool& pool,
                           atomic_uint& pos)
    : GOThread(),
      m_Objects(objs),
      m_Pos(pos),
      m_pool(pool),
      m_Error(),
      m_OutOfMemory(false) {}

GOLoadThread::~GOLoadThread() { Stop(); }

void GOLoadThread::checkResult() {
  Wait();
  if (m_Error != wxEmptyString) throw m_Error;
  if (m_OutOfMemory) throw GOOutOfMemory();
}

void GOLoadThread::Run() { Start(); }

void GOLoadThread::Entry() {
  while (!ShouldStop()) {
    if (m_pool.IsPoolFull()) return;
    unsigned pos = m_Pos.fetch_add(1);
    try {
      GOCacheObject* obj = m_Objects.GetCacheObject(pos);
      if (!obj) return;
      obj->LoadData();
    } catch (GOOutOfMemory e) {
      m_OutOfMemory = true;
      return;
    } catch (wxString error) {
      m_Error = error;
      return;
    }
  }
  return;
}
