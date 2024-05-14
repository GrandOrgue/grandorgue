/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoadWorker.h"

#include "model/GOCacheObject.h"

#include "GOAlloc.h"
#include "GOCacheObjectDistributor.h"
#include "GOMemoryPool.h"

GOLoadWorker::GOLoadWorker(
  const GOFileStore &fileStore,
  GOMemoryPool &pool,
  GOCacheObjectDistributor &distributor)
  : m_FileStore(fileStore),
    m_pool(pool),
    m_distributor(distributor),
    m_WereExceptions(false),
    m_OutOfMemory(false) {}

void GOLoadWorker::LoadObjectNoExc(GOCacheObject *obj) {
  try {
    m_WereExceptions |= !obj->LoadFromFileWithoutExc(m_FileStore, m_pool);
  } catch (GOOutOfMemory e) {
    m_OutOfMemory = true;
    m_WereExceptions = true;
  }
}

bool GOLoadWorker::LoadNextObject(GOCacheObject *&obj) {
  if (
    !m_OutOfMemory && !m_pool.IsPoolFull() && (obj = m_distributor.FetchNext()))
    LoadObjectNoExc(obj);
  return obj && !m_OutOfMemory;
}

bool GOLoadWorker::WereExceptions() const {
  if (m_OutOfMemory)
    throw GOOutOfMemory();
  return m_WereExceptions;
}
