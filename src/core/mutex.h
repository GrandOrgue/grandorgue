/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMUTEX_H
#define GORGUEMUTEX_H

#include "GOLock.h"
#ifdef WX_MUTEX
#include <wx/thread.h>
#else
#include "atomic.h"
#endif

class GOMutex
{
private:
#ifdef WX_MUTEX
	wxMutex m_Mutex;

	void Init()
	{
	}

	void Cleanup()
	{
	}

	void DoLock()
	{
		m_Mutex.Lock();
	}

	void DoUnlock()
	{
		m_Mutex.Unlock();
	}

	bool DoTryLock()
	{
		return m_Mutex.TryLock() == wxMUTEX_NO_ERROR;
	}
	friend class GOCondition;
#else
	GOWaitQueue m_Wait;
	atomic_int m_Lock;

	void Init()
	{
		m_Lock = 0;
	}

	void Cleanup()
	{
	}

	void DoLock()
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

	void DoUnlock()
	{
		__sync_synchronize();
		int value = m_Lock.fetch_add(-1);
		if (value > 1)
			m_Wait.Wakeup();
	}

	bool DoTryLock()
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

	GOMutex(const GOMutex&) = delete;
	const GOMutex& operator=(const GOMutex&) = delete;

public:
	GOMutex()
	{
		Init();
	}

	~GOMutex()
	{
		Cleanup();
	}

	void Lock()
	{
		DoLock();
	}

	void Unlock()
	{
		DoUnlock();
	}

	bool TryLock()
	{
		return DoTryLock();
	}
};

class GOCondition
{
private:
#ifdef WX_MUTEX
	wxCondition m_condition;

	void Init()
	{
	}

	void Cleanup()
	{
	}

	void DoWait()
	{
		m_condition.Wait();
	}

	void DoSignal()
	{
		m_condition.Signal();
	}

	void DoBroadcast()
	{
		m_condition.Broadcast();
	}
#else
	atomic_int m_Waiters;
	GOWaitQueue m_Wait;

	void Init()
	{
		m_Waiters = 0;
	}

	void Cleanup()
	{
		while(m_Waiters > 0)
			Signal();
	}

	void DoWait()
	{
		m_Waiters.fetch_add(1);
		m_Mutex.Unlock();
		m_Wait.Wait();
		m_Mutex.Lock();
	}

	void DoSignal()
	{
		int waiters = m_Waiters.fetch_add(-1);
		if (waiters <= 0)
			m_Waiters.fetch_add(+1);
		else
			m_Wait.Wakeup();
	}

	void DoBroadcast()
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
	GOMutex& m_Mutex;

	GOCondition(const GOCondition&) = delete;
	const GOCondition& operator=(const GOCondition&) = delete;
public:
	GOCondition(GOMutex& mutex) :
#ifdef WX_MUTEX
		m_condition(mutex.m_Mutex),
#endif
		m_Mutex(mutex)
	{
		Init();
	}

	~GOCondition()
	{
		Cleanup();
	}

	void Wait()
	{
		DoWait();
	}

	void Signal()
	{
		DoSignal();
	}

	void Broadcast()
	{
		DoBroadcast();
	}
};

#endif
