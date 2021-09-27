/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPEWINDCHESTCALLBACK_H
#define GORGUEPIPEWINDCHESTCALLBACK_H

class GOrguePipeWindchestCallback
{
 public:
	virtual ~GOrguePipeWindchestCallback()
	{
	}

	virtual void SetTremulant(bool on) = 0;
};

#endif
