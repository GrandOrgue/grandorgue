/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoadThread.h"

#include "model/GOCacheObject.h"

#include "GOAlloc.h"
#include "GOCacheObjectDistributor.h"
#include "GOMemoryPool.h"

GOLoadThread::GOLoadThread(
  GOMemoryPool &pool, GOCacheObjectDistributor &distributor)
  : GOThread(),
    m_pool(pool),
    m_distributor(distributor),
    m_Error(),
    m_OutOfMemory(false) {}

GOLoadThread::~GOLoadThread() { Stop(); }

void GOLoadThread::checkResult() {
  Wait();
  if (m_Error != wxEmptyString)
    throw m_Error;
  if (m_OutOfMemory)
    throw GOOutOfMemory();
}

void GOLoadThread::Run() { Start(); }

void GOLoadThread::Entry() {
  while (!ShouldStop()) {
    if (m_pool.IsPoolFull())
      return;
    try {
      GOCacheObject *obj = m_distributor.fetchNext();

      if (!obj)
        return;
      obj->LoadData(m_pool);
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
