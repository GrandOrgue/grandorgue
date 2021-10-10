/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOCONDITION_H
#define GOCONDITION_H

#ifdef WX_MUTEX
#include <wx/thread.h>
#else
#include "atomic.h"
#include "GOWaitQueue.h"
#endif

#include "GOMutex.h"

class GOCondition
{
private:
#ifdef WX_MUTEX
  wxCondition m_condition;
#else
  atomic_int m_Waiters;
  GOWaitQueue m_Wait;
#endif
  GOMutex& m_Mutex;

  GOCondition(const GOCondition&) = delete;
  const GOCondition& operator=(const GOCondition&) = delete;
public:
  GOCondition(GOMutex& mutex);
  ~GOCondition();

  void Wait();
  void Signal();
  void Broadcast();
};

#endif /* GOCONDITION_H */
