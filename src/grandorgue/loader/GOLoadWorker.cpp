/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOLoadWorker.h"

#include <wx/intl.h>

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
    m_HasBeenException(false),
    m_OutOfMemory(false) {}

bool GOLoadWorker::LoadNextObject(GOCacheObject *&obj) {
  bool isLoaded = false;

  if (
    !m_HasBeenException && !m_pool.IsPoolFull()
    && (obj = m_distributor.fetchNext())) {
    assert(m_errMsg.IsEmpty()); // otherwise m_HasBeenException
    try {
      obj->LoadData(m_FileStore, m_pool);
      isLoaded = true;
    } catch (GOOutOfMemory e) {
      m_OutOfMemory = true;
    } catch (wxString error) {
      m_errMsg = error;
    } catch (const std::exception &e) {
      m_errMsg = e.what();
    } catch (...) { // We must not allow unhandled exceptions here
      m_errMsg = _("Unknown exception");
    }
    if (!isLoaded) {
      m_HasBeenException = true;
      // add the object title to the error message
      if (!m_errMsg.IsEmpty())
        m_errMsg.Printf(
          _("Unable to load %s: %s"), obj->GetLoadTitle(), m_errMsg);
    }
  }
  return isLoaded;
}

void GOLoadWorker::AssertNoException() const {
  if (!m_errMsg.IsEmpty()) {
    throw m_errMsg;
  }
  if (m_OutOfMemory)
    throw GOOutOfMemory();
}
