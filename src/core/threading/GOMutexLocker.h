/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOUTEXLOCKER_H
#define GOMUTEXLOCKER_H

#include "GOMutex.h"

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

	bool TryLock()
	{
		if (!m_Locked)
			m_Locked = m_Mutex.TryLock();
		return m_Locked;
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

	GOMutexLocker(const GOMutexLocker&) = delete;
	GOMutexLocker& operator=(const GOMutexLocker&) = delete;
};

#endif
