/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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
 */

#include "GOrgueThread.h"

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

void GOrgueThread::Wait()
{
	if (m_Thread.joinable())
		m_Thread.join();
}

void GOrgueThread::Stop()
{
	m_Stop = true;
	Wait();
}

bool GOrgueThread::ShouldStop()
{
	return m_Stop;
}

void GOrgueThread::EntryPoint(GOrgueThread* thread)
{
	thread->Entry();
}
