/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOMUTEXLOCKER_H
#define GOMUTEXLOCKER_H

#include "GOMutex.h"

class GOMutexLocker
{
private:
	GOMutex& m_Mutex;
	bool m_Locked;
public:
	/**
	 * Lock the mutex
	 * When try_lock = true or pThread is not null then the mutex may be not locked
	 * after invoking and the caller must check with IsLocked().
	 * Otherwise the mutex is locked after return and the caller may not worry about it
	 *
	 * @param mutex: mutex to lock
	 * @param try_lock: if true and the mutex is ocuppied then returns immediate
	 * @param lockerInfo an information who locks the mutex. Useful for diagnostic deadlocks
	 * @param pThread. If != NULL then the constructor may return wihout locking when pThread->ShouldStop()
	 */

	GOMutexLocker(GOMutex& mutex, bool try_lock = false, const char* lockerInfo = NULL, GOThread* pThread = NULL) :
		m_Mutex(mutex),
		m_Locked(false)
	{
		if (try_lock)
			m_Locked = m_Mutex.TryLock(lockerInfo);
		else if (pThread != NULL)
			m_Locked = m_Mutex.LockOrStop(lockerInfo, pThread);
		else
		{
			m_Mutex.Lock(lockerInfo);
			m_Locked = true;
		}
	}

	~GOMutexLocker()
	{
		if (m_Locked)
			m_Mutex.Unlock();
	}

	bool TryLock(const char* lockerInfo = NULL)
	{
		if (!m_Locked)
			m_Locked = m_Mutex.TryLock(lockerInfo);
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
