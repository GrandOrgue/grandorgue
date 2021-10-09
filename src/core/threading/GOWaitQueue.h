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

class GOWaitQueue
{
private:
	wxSemaphore m_Wait;
public:
	GOWaitQueue()
	{
	}

	~GOWaitQueue()
	{
	}

	void Wait()
	{
		m_Wait.Wait();
	}

	void Wakeup()
	{
		m_Wait.Post();
	}
};
#else

#include <mutex>

class GOWaitQueue
{
private:
	std::mutex m_Wait;
public:
	GOWaitQueue()
	{
		m_Wait.lock();
	}

	~GOWaitQueue()
	{
		m_Wait.unlock();
	}

	void Wait()
	{
		m_Wait.lock();
	}

	void Wakeup()
	{
		m_Wait.unlock();
	}
};
#endif

#endif
