/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUETIMER_H
#define GORGUETIMER_H

#include "GOrgueTime.h"
#include "mutex.h"
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
