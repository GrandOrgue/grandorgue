/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueThread.h"

#include "atomic.h"

GOrgueThread::GOrgueThread() :
	m_Thread(),
	m_Stop(false)
{
}

GOrgueThread::~GOrgueThread()
{
	Stop();
}

void GOrgueThread::Start()
{
	m_Stop = false;
	if (m_Thread.joinable())
		return;
	m_Thread = std::thread(GOrgueThread::EntryPoint, this);
}

void GOrgueThread::MarkForStop()
{
	m_Stop = true;
}

void GOrgueThread::Wait()
{
	if (m_Thread.joinable())
		m_Thread.join();
}

void GOrgueThread::Stop()
{
	MarkForStop();
	Wait();
}

bool GOrgueThread::ShouldStop()
{
	return load_once(m_Stop);
}

void GOrgueThread::EntryPoint(GOrgueThread* thread)
{
	thread->Entry();
}
