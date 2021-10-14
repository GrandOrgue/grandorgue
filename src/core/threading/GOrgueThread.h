/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETHREAD_H
#define GORGUETHREAD_H

#include <thread>

class GOrgueThread
{
private:
	std::thread m_Thread;
	bool m_Stop;

	static void EntryPoint(GOrgueThread* thread);
	
protected:
	virtual void Entry() = 0;

public:
	GOrgueThread();
	virtual ~GOrgueThread();

	void Start();
	void MarkForStop();
	void Wait();
	void Stop();
	
	bool ShouldStop();
};

#endif
