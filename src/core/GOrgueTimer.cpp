/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueTimer.h"

#include "GOrgueTimerCallback.h"
#include "threading/GOMutexLocker.h"

GOrgueTimer::GOrgueTimer() :
	wxTimer(),
	m_Entries()
{
}

GOrgueTimer::~GOrgueTimer()
{
	wxTimer::Stop();
}

void GOrgueTimer::SetRelativeTimer(GOTime time, GOrgueTimerCallback* callback, unsigned interval)
{
	time += wxGetLocalTimeMillis();
	SetTimer(time, callback, interval);
}

void GOrgueTimer::SetTimer(GOTime time, GOrgueTimerCallback* callback, unsigned interval)
{
	GOrgueTimerEntry e;
	e.time = time;
	e.callback = callback;
	e.interval = interval;
	bool added = false;
	for(unsigned i = 0; !added && i < m_Entries.size(); i++)
		if (m_Entries[i].callback == NULL)
		{
			m_Entries[i] = e;
			added = true;
		}
	if (!added)
		m_Entries.push_back(e);

	GOMutexLocker locker(m_Lock, true);
	if (locker.IsLocked())
		Schedule();
}

void GOrgueTimer::UpdateInterval(GOrgueTimerCallback* callback, unsigned interval)
{
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].callback == callback)
			m_Entries[i].interval = interval;
}


void GOrgueTimer::DeleteTimer(GOrgueTimerCallback* callback)
{
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].callback == callback)
			m_Entries[i].callback = NULL;
}

void GOrgueTimer::Schedule()
{
	GOTime next = -1;
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].callback)
		{
			if (next == -1 || next > m_Entries[i].time)
				next = m_Entries[i].time;
		}
	if (next != -1)
	{
		GOTime now = wxGetLocalTimeMillis();
		GOTime wait;
		if (next > now)
			wait = next - now;
		else
			wait = 1;
		wxTimer::Start(wait.GetValue(), true);
	}
}

void GOrgueTimer::Notify()
{
	GOMutexLocker locker(m_Lock);

	GOTime now = wxGetLocalTimeMillis();
	for(unsigned i = 0; i < m_Entries.size(); i++)
		if (m_Entries[i].callback && m_Entries[i].time <= now)
		{
			m_Entries[i].callback->HandleTimer();
			if (m_Entries[i].interval)
			{
				if (m_Entries[i].time <= now + m_Entries[i].interval)
					m_Entries[i].time = now;
				m_Entries[i].time += m_Entries[i].interval;
			}
			else
				m_Entries[i].callback = NULL;
		}
	Schedule();
}
