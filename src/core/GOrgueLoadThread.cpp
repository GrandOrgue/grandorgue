/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueLoadThread.h"

#include "GOrgueAlloc.h"
#include "GOrgueCacheObject.h"
#include "GOrgueEventDistributor.h"
#include "GOrgueMemoryPool.h"

GOrgueLoadThread::GOrgueLoadThread(GOrgueEventDistributor& objs, GOrgueMemoryPool& pool, atomic_uint& pos) :
	GOrgueThread(),
	m_Objects(objs),
	m_Pos(pos),
	m_pool(pool),
	m_Error(),
	m_OutOfMemory(false)
{
}

GOrgueLoadThread::~GOrgueLoadThread()
{
	Stop();
}

void GOrgueLoadThread::checkResult()
{
	Wait();
	if (m_Error != wxEmptyString)
		throw m_Error;
	if (m_OutOfMemory)
		throw GOrgueOutOfMemory();
}

void GOrgueLoadThread::Run()
{
	Start();
}

void GOrgueLoadThread::Entry()
{
	while (!ShouldStop())
	{
		if (m_pool.IsPoolFull())
			return;
		unsigned pos = m_Pos.fetch_add(1);
		try
		{
			GOrgueCacheObject* obj = m_Objects.GetCacheObject(pos);
			if (!obj)
				return;
			obj->LoadData();
		}
		catch (GOrgueOutOfMemory e)
		{
			m_OutOfMemory = true;
			return;
		}
		catch (wxString error)
		{
			m_Error = error;
			return;
		}
	}
	return;
}
