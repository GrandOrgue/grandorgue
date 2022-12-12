/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
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
  bool m_HasBeenException; // any exception included GOOutOfMemory
  bool m_OutOfMemory;
  wxString m_errMsg;

public:
  /**
   * Constructs a GOLoadWorker object
   * @param distributor - the instacne of GOCacheObjectDistributor to take
   *  objects from
   * @param fileStore - passed to GOCacheObject::LoadData
   * @param pool - passed to GOCacheObject::LoadData
   */
  GOLoadWorker(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    GOCacheObjectDistributor &distributor);

  /**
   * Takes a next object from m_distributor that has not been taken by any
   *   worker and loads it. If an exception occured then remembers it for
   *   future calls of AssertNoException()
   * @param obj - the object tried to be loaded
   * @return true if the object has been loaded successfully
   *   false if there are no more objects to load or there was an exception
   */
  bool LoadNextObject(GOCacheObject *&obj);

  /**
   * If there was an exception in LoadNextObject then throws wxString with the
   *   error message or GOOutOfMemory
   * Otherwise does nothing
   */
  void AssertNoException() const;
};

#endif /* GOLOADWORKER_H */
