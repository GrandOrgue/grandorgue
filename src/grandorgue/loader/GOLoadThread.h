/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADTHREAD_H
#define GOLOADTHREAD_H

#include <wx/string.h>

#include "threading/GOThread.h"

#include "GOCacheObjectDistributor.h"

class GOMemoryPool;

class GOLoadThread : private GOThread {
private:
  GOMemoryPool &m_pool;
  GOCacheObjectDistributor &m_distributor;
  wxString m_Error;
  bool m_OutOfMemory;

  void Entry();

public:
  GOLoadThread(GOMemoryPool &pool, GOCacheObjectDistributor &distributor);
  ~GOLoadThread();

  void Run();
  void checkResult();
};

#endif
