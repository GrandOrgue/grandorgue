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

class GOMutexLocker
{
private:
	wxMutex& m_Mutex;
	bool m_Locked;
public:
	GOMutexLocker(wxMutex& mutex, bool try_lock = false) :
		m_Mutex(mutex),
		m_Locked(false)
	{
		if (try_lock)
			m_Locked = m_Mutex.TryLock() == wxMUTEX_NO_ERROR;
		else
			m_Locked = m_Mutex.Lock() == wxMUTEX_NO_ERROR;
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
};

class GOMultiMutexLocker
{
private:
	std::vector<wxMutex*> m_Mutex;

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

	void Add(wxMutex& mutex)
	{
		mutex.Lock();
		m_Mutex.push_back(&mutex);
	}
};

#endif
