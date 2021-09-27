/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDICALLBACK_H
#define GORGUEMIDICALLBACK_H

class GOrgueMidiEvent;

class GOrgueMidiCallback
{
public:
	virtual ~GOrgueMidiCallback()
	{
	}

	
	virtual void OnMidiEvent(const GOrgueMidiEvent& event) = 0;
};

#endif
