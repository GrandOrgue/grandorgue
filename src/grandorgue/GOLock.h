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
	wxMutex m_Mutex;

	void Init()
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

	GOMutex(const GOMutex&);
	const GOMutex& operator=(const GOMutex&);

public:
	GOMutex()
	{
		Init();
	}

	~GOMutex()
	{
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
