/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueLoadThread.h"

#include "GOrgueCacheObject.h"
#include "GOrgueMemoryPool.h"

GOrgueLoadThread::GOrgueLoadThread(std::vector<GOrgueCacheObject*>& objs, std::atomic_uint& pos) :
	wxThread(wxTHREAD_JOINABLE),
	m_Objects(objs),
	m_Pos(pos),
	m_Error(),
	m_OutOfMemory(false)
{
	Create();
}

GOrgueLoadThread::~GOrgueLoadThread()
{
	Delete();
}

void GOrgueLoadThread::checkResult()
{
	Wait();
	if (m_Error != wxEmptyString)
		throw m_Error;
	if (m_OutOfMemory)
		throw GOrgueOutOfMemory();
}

void* GOrgueLoadThread::Entry()
{
	while (!TestDestroy())
	{
		unsigned pos = m_Pos.fetch_add(1);
		if (pos >= m_Objects.size())
			return NULL;
		try
		{
			GOrgueCacheObject* obj = m_Objects[pos];
			obj->LoadData();
		}
		catch (GOrgueOutOfMemory e)
		{
			m_OutOfMemory = true;
			return NULL;
		}
		catch (wxString error)
		{
			m_Error = error;
			return NULL;
		}
	}
	return NULL;
}
