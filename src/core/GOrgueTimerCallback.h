/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETIMERCALLBACK_H
#define GORGUETIMERCALLBACK_H

class GOrgueTimerCallback
{
public:
	~GOrgueTimerCallback()
	{
	}

	virtual void HandleTimer() = 0;
};

#endif
