/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADTHREAD_H
#define GOLOADTHREAD_H

#include "threading/GOThread.h"

#include "GOLoadWorker.h"

class GOLoadThread : private GOLoadWorker, private GOThread {
private:
  /* the main loading loop. It takes objects from the m_CacheObjects
   * concurrently with other threads loads them
   */
  void Entry() override;

public:
  GOLoadThread(
    const GOFileStore &fileStore,
    GOMemoryPool &pool,
    GOCacheObjectDistributor &distributor)
    : GOLoadWorker(fileStore, pool, distributor) {}
  ~GOLoadThread() { Stop(); }

  void Run() { Start(); }

  /**
   * Waits for completions
   * @return true if any exceptions occured. Otherwise - false
   */
  bool CheckExceptions();
};

#endif
