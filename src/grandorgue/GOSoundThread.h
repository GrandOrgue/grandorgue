/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDTHREAD_H
#define GOSOUNDTHREAD_H

#include "threading/GOCondition.h"
#include "threading/GOMutex.h"
#include "threading/GOrgueThread.h"

class GOSoundScheduler;

class GOSoundThread : public GOrgueThread
{
private:
	GOSoundScheduler* m_Scheduler;

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
