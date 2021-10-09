/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUELOADTHREAD_H
#define GORGUELOADTHREAD_H

#include "threading/atomic.h"
#include "threading/GOrgueThread.h"
#include <wx/string.h>
#include <wx/thread.h>

class GOrgueCacheObject;
class GOrgueEventDistributor;
class GOrgueMemoryPool;

class GOrgueLoadThread : private GOrgueThread
{
private:
	GOrgueEventDistributor& m_Objects;
	atomic_uint& m_Pos;
	GOrgueMemoryPool& m_pool;
	wxString m_Error;
	bool m_OutOfMemory;

	void Entry();

public:
	GOrgueLoadThread(GOrgueEventDistributor& objs, GOrgueMemoryPool& pool, atomic_uint& pos);
	~GOrgueLoadThread();

	void Run();
	void checkResult();
};

#endif
