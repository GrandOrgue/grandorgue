/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOWAITQUEUE_H
#define GOWAITQUEUE_H

#if 1
//#ifdef __WIN32__
  #define GOWAITQUEUE_USE_WX
  #undef GOWAITQUEUE_USE_STD_MUTEX

  #include <wx/thread.h>

#else

  #include <mutex>

  #undef GOWAITQUEUE_USE_WX
  #define GOWAITQUEUE_USE_STD_MUTEX
#endif

class GOWaitQueue
{
private:
  
#ifdef GOWAITQUEUE_USE_WX
  wxSemaphore m_Wait;
#elif defined(GOWAITQUEUE_USE_STD_MUTEX)
  std::mutex m_Wait;
#endif
  
public:
  GOWaitQueue();
  ~GOWaitQueue();

  void Wait();
  void Wakeup();
};

#endif
