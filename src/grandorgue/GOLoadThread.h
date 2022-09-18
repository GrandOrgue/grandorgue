/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOADTHREAD_H
#define GOLOADTHREAD_H

#include <wx/string.h>
#include <wx/thread.h>

#include "threading/GOThread.h"
#include "threading/atomic.h"

class GOCacheObject;
class GOEventDistributor;
class GOMemoryPool;

class GOLoadThread : private GOThread {
private:
  GOEventDistributor &m_Objects;
  atomic_uint &m_Pos;
  GOMemoryPool &m_pool;
  wxString m_Error;
  bool m_OutOfMemory;

  void Entry();

public:
  GOLoadThread(GOEventDistributor &objs, GOMemoryPool &pool, atomic_uint &pos);
  ~GOLoadThread();

  void Run();
  void checkResult();
};

#endif
