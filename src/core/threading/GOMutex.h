/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMUTEX_H
#define GOMUTEX_H

#ifdef WX_MUTEX
#include <wx/thread.h>
#else
#include "atomic.h"
#include "GOWaitQueue.h"
#endif

class GOMutex
{
private:
#ifdef WX_MUTEX
  friend class GOCondition;

  wxMutex m_Mutex;
#else
  GOWaitQueue m_Wait;
  atomic_int m_Lock;
#endif

  GOMutex(const GOMutex&) = delete;
  const GOMutex& operator=(const GOMutex&) = delete;

public:
  GOMutex();
  ~GOMutex();

  void Lock();
  void Unlock();
  bool TryLock();
};

#endif /* GOMUTEX_H */

