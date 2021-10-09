#include "GOCondition.h"

GOCondition::GOCondition(GOMutex& mutex) :
#ifdef WX_MUTEX
	m_condition(mutex.m_Mutex),
#else
	m_Waiters(0),
#endif
	m_Mutex(mutex)
{
}

#ifdef WX_MUTEX

GOCondition::~GOCondition()
{
}

void GOCondition::Wait()
{
	m_condition.Wait();
}

void GOCondition::Signal()
{
	m_condition.Signal();
}

void GOCondition::Broadcast()
{
	m_condition.Broadcast();
}

#else

GOCondition::~GOCondition()
{
  while(m_Waiters > 0)
	  Signal();
}

void GOCondition::Wait()
{
	m_Waiters.fetch_add(1);
	m_Mutex.Unlock();
	m_Wait.Wait();
	m_Mutex.Lock();
}

void GOCondition::Signal()
{
	int waiters = m_Waiters.fetch_add(-1);
	if (waiters <= 0)
		m_Waiters.fetch_add(+1);
	else
		m_Wait.Wakeup();
}

void GOCondition::Broadcast()
{
	int waiters;
	do
	{
		waiters = m_Waiters.fetch_add(-1);
		if (waiters <= 0)
		{
			m_Waiters.fetch_add(+1);
			return;
		}
		else
			m_Wait.Wakeup();
	}
	while(waiters > 1);
}

#endif
