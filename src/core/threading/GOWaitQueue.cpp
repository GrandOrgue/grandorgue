/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

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

void GOWaitQueue::Wait()
{
#ifdef GOWAITQUEUE_USE_WX
  m_Wait.Wait();
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  m_Wait.lock();
#endif
}

void GOWaitQueue::Wakeup()
{
#ifdef GOWAITQUEUE_USE_WX
  m_Wait.Post();
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  m_Wait.unlock();
#endif
}
