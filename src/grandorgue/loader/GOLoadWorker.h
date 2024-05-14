/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCacheObjectDistributor.h"

#include <wx/string.h>

class GOFileStore;
class GOMemoryPool;

#ifndef GOLOADWORKER_H
#define GOLOADWORKER_H

/**
 * A class for loading objects taken from GOCacheObjectDistributor
 * Usually several instances of GOLoadWorker are loading objects from one common
 * GOCacheObjectDistributor instance in parallel
 */

class GOLoadWorker {
private:
  const GOFileStore &m_FileStore;
  GOMemoryPool &m_pool;
  GOCacheObjectDistributor &m_distributor;

  GOCacheObject *m_LastObject;
  bool m_WereExceptions; // any exception included GOOutOfMemory
  bool m_OutOfMemory;

public:
  /**
   * Constructs a GOLoadWorker object
   * @param distributor - the instance of GOCacheObjectDistributor to take
   *  objects from
   * @param fileStore - passed to GOCacheObject::LoadData
   * @param pool - passed to GOCacheObject::LoadData
   */
  GOLoadWorker(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    GOCacheObjectDistributor &distributor);

  /**
   * Load the object. If an exception occured then remembers it for
   *   future calls of AssertNoException(). Only GOOutOfMemory may be thrown,
   *   all other exceptions are catched and remembered  for
   *   future calls of AssertNoException()
   * @param obj - the object to loaded
   */
  void LoadObjectNoExc(GOCacheObject *obj);

  /**
   * Takes a next object from m_distributor that has not been taken by any
   *   worker and loads it with LoadObjectNoExc.
   * @param obj - the object tried to be loaded
   * @return true if The loading may continue
   *   false if there are no more objects to load or there was GOOutOfMemory
   */
  bool LoadNextObject(GOCacheObject *&obj);

  /**
   * If there was GOOutOfMemory then rethrows it
   * Otherwise does nothing
   * @return whether any exceptions were occured
   */
  bool WereExceptions() const;
};

#endif /* GOLOADWORKER_H */
