/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSoundThread.h"

#include "GOSoundScheduler.h"
#include "GOSoundWorkItem.h"
#include "threading/GOMutexLocker.h"
#include <wx/log.h>

GOSoundThread::GOSoundThread(GOSoundScheduler* scheduler):
	GOrgueThread(),
	m_Scheduler(scheduler),
	m_Stop(false),
	m_Condition(m_Mutex)
{
	wxLogDebug(wxT("Create Thread"));
}

void GOSoundThread::Entry()
{
	while(!ShouldStop() && !m_Stop)
	{
		GOSoundWorkItem *next;
		do
		{
			next = m_Scheduler->GetNextGroup();
			if (next != NULL)
				next->Run();
		}
		while (next != NULL);

		GOMutexLocker lock(m_Mutex);
		if (ShouldStop() || m_Stop)
			break;
		m_Condition.Wait();
	}
	return;
}

void GOSoundThread::Run()
{
	Start();
}

void GOSoundThread::Wakeup()
{
	m_Condition.Signal();
}

void GOSoundThread::Delete()
{
	{
		GOMutexLocker lock(m_Mutex);
		m_Stop = true;
		m_Condition.Signal();
	}
	Stop();
}
