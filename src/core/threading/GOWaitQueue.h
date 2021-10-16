/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOWAITQUEUE_H
#define GOWAITQUEUE_H

#if 1
//#ifdef __WIN32__
  #include <wx/thread.h>

  #define GOWAITQUEUE_USE_WX
  #undef GOWAITQUEUE_USE_STD_MUTEX
#else
  #include <mutex>

  #undef GOWAITQUEUE_USE_WX
  #define GOWAITQUEUE_USE_STD_MUTEX
#endif

#include "GOrgueThread.h"

class GOWaitQueue
{
private:
  
#ifdef GOWAITQUEUE_USE_WX
  wxSemaphore m_Wait;
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  std::timed_mutex m_Wait;
#endif

  void WaitInfinitelly();
  bool WaitWithTimeout();

public:
  GOWaitQueue();
  ~GOWaitQueue();

  /**
   * Wait until a signal is received or a timeout is occured.
   * The timeout value is defined in threading_impl.h.
   * @param isWithTimeout - if true then to wait with timeout. If false else to wait infinitely
   * @return true if a signal received and false if the timeout occured
   */
  bool Wait(bool isWithTimeout);

  void Wakeup();
};

#endif
