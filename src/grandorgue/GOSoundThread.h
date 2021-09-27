/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDTHREAD_H
#define GOSOUNDTHREAD_H

#include "GOrgueThread.h"
#include "mutex.h"

class GOSoundScheduler;

class GOSoundThread : private GOrgueThread
{
private:
	GOSoundScheduler* m_Scheduler;
	bool m_Stop;

	GOMutex m_Mutex;
	GOCondition m_Condition;

	void Entry();

public:
	GOSoundThread(GOSoundScheduler* scheduler);

	void Run();
	void Delete();
	void Wakeup();
};

#endif
