#include "GOMutex.h"

#define GO_PRINTCONTENTION 0

#ifdef WX_MUTEX

void GOMutex::GOMutex()
{
}

void GOMutex::~GOMutex()
{
}

void GOMutex::Lock()
{
	m_Mutex.Lock();
}

void GOMutex::Unlock()
{
	m_Mutex.Unlock();
}

bool GOMutex::TryLock()
{
	return m_Mutex.TryLock() == wxMUTEX_NO_ERROR;
}

#else

GOMutex::GOMutex()
{
	m_Lock = 0;
}

GOMutex::~GOMutex()
{
}

void GOMutex::Lock()
{
  int value = m_Lock.fetch_add(1);
  
  if (!value)
  {
	  __sync_synchronize();
	  return;
  }
#if GO_PRINTCONTENTION
  wxLogWarning(wxT("Mutex::wait %p"), this);
  GOStackPrinter::printStack(this);
#endif

  m_Wait.Wait();
#if GO_PRINTCONTENTION
  wxLogWarning(wxT("Mutex::end_wait %p"), this);
#endif
}

void GOMutex::Unlock()
{
  __sync_synchronize();
  
  int value = m_Lock.fetch_add(-1);
  
  if (value > 1)
	  m_Wait.Wakeup();
}

bool GOMutex::TryLock()
{
  int old = 0;
  
  if (m_Lock.compare_exchange(old, 1))
  {
	  __sync_synchronize();
	  return true;
  }
  return false;
}

#endif
