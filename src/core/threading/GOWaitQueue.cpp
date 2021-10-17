/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifdef GOWAITQUEUE_USE_STD_MUTEX
#include <chrono>
#endif

#include "threading_impl.h"
#include "GOWaitQueue.h"

GOWaitQueue::GOWaitQueue()
{
#ifdef GOWAITQUEUE_USE_STD_MUTEX
  m_Wait.lock();
#endif
}

GOWaitQueue::~GOWaitQueue()
{
#ifdef GOWAITQUEUE_USE_STD_MUTEX
  m_Wait.unlock();
#endif
}

void GOWaitQueue::WaitInfinitely()
{
#ifdef GOWAITQUEUE_USE_WX
  m_Wait.Wait();
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  m_Wait.lock();
#endif
}

bool GOWaitQueue::WaitWithTimeout()
{
#ifdef GOWAITQUEUE_USE_WX
  return m_Wait.WaitTimeout(WAIT_TIMEOUT_MS) != wxSEMA_TIMEOUT;
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  return m_Wait.try_lock_for(timeOutMs);
#endif
}

bool GOWaitQueue::Wait(bool isWithTimeout)
{
  bool rc;

  if (isWithTimeout)
    rc = WaitWithTimeout();
  else
  {
    WaitInfinitely();
    rc = true;
  }
  return rc;
}


void GOWaitQueue::Wakeup()
{
#ifdef GOWAITQUEUE_USE_WX
  m_Wait.Post();
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  m_Wait.unlock();
#endif
}
