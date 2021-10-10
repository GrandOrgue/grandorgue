/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETIMER_H
#define GORGUETIMER_H

#include "GOrgueTime.h"
#include "threading/GOMutex.h"
#include <wx/timer.h>
#include <vector>

class GOrgueTimerCallback;

class GOrgueTimer : private wxTimer
{
	typedef struct
	{
		GOTime time;
		GOrgueTimerCallback* callback;
	        unsigned interval;
	} GOrgueTimerEntry;
private:
	std::vector<GOrgueTimerEntry> m_Entries;
	GOMutex m_Lock;

 	void Schedule();
	void Notify();

public:
	GOrgueTimer();
	~GOrgueTimer();

	void SetTimer(GOTime time, GOrgueTimerCallback* callback, unsigned interval = 0);
	void SetRelativeTimer(GOTime time, GOrgueTimerCallback* callback, unsigned interval = 0);
	void UpdateInterval(GOrgueTimerCallback* callback, unsigned interval);
	void DeleteTimer(GOrgueTimerCallback* callback);
};

#endif
