/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * MA 02111-1307, USA.
 */

#ifndef GOLOCK_H
#define GOLOCK_H

#include <wx/wx.h>
#include <vector>
#include "GrandOrgueDef.h"

class GOMutex
{
private:
#ifndef HAVE_ATOMIC
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
	wxCriticalSection m_Mutex;
	int m_Lock;

	void Init()
	{
		m_Lock = 0;
		m_Mutex.Enter();
	}

	void Cleanup()
	{
		m_Mutex.Leave();
	}

	void DoLock()
	{
		int value = __sync_fetch_and_add(&m_Lock, 1);
		if (!value)
		{
			__sync_synchronize();
			return;
		}
		m_Mutex.Enter();
	}

	void DoUnlock()
	{
		__sync_synchronize();
		int value = __sync_fetch_and_add(&m_Lock, -1);
		if (value > 1)
			m_Mutex.Leave();
	}

	bool DoTryLock()
	{
		if (__sync_bool_compare_and_swap(&m_Lock, 0, 1))
		{
			__sync_synchronize();
			return true;
		}
		return false;
	}
#endif

	GOMutex(const GOMutex&);
	const GOMutex& operator=(const GOMutex&);

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
#ifndef HAVE_ATOMIC
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
#else
	int m_Waiters;
	wxCriticalSection m_Wait;

	void Init()
	{
		m_Waiters = 0;
		m_Wait.Enter();
	}

	void Cleanup()
	{
		m_Wait.Leave();
	}

	void DoWait()
	{
		__sync_fetch_and_add(&m_Waiters, 1);
		m_Mutex.Unlock();
		m_Wait.Enter();
		m_Mutex.Lock();
	}

	void DoSignal()
	{
		int waiters = __sync_fetch_and_add(&m_Waiters, -1);
		if (waiters <= 0)
			__sync_fetch_and_add(&m_Waiters, +1);
		else
			m_Wait.Leave();
	}

#endif
	GOMutex& m_Mutex;

	GOCondition(const GOCondition&);
	const GOCondition& operator=(const GOCondition&);
public:
	GOCondition(GOMutex& mutex) :
#ifndef HAVE_ATOMIC
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
};

class GOMutexLocker
{
private:
	GOMutex& m_Mutex;
	bool m_Locked;
public:
	GOMutexLocker(GOMutex& mutex, bool try_lock = false) :
		m_Mutex(mutex),
		m_Locked(false)
	{
		if (try_lock)
			m_Locked = m_Mutex.TryLock();
		else
		{
			m_Mutex.Lock();
			m_Locked = true;
		}
	}

	~GOMutexLocker()
	{
		if (m_Locked)
			m_Mutex.Unlock();
	}

	bool IsLocked() const
	{
		return m_Locked;
	}

	void Unlock()
	{
		if (m_Locked)
			m_Mutex.Unlock();
		m_Locked = false;
	}
};

class GOMultiMutexLocker
{
private:
	std::vector<GOMutex*> m_Mutex;

public:
	GOMultiMutexLocker() :
		m_Mutex(0)
	{
	}

	~GOMultiMutexLocker()
	{
		while(m_Mutex.size())
		{
			m_Mutex.back()->Unlock();
			m_Mutex.pop_back();
		}
	}

	void Add(GOMutex& mutex)
	{
		mutex.Lock();
		m_Mutex.push_back(&mutex);
	}
};

#endif
