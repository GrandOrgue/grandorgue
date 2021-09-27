/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEEVENTHANDLER_H
#define GORGUEEVENTHANDLER_H

class GOrgueMidiEvent;

class GOrgueEventHandler
{
public:
	virtual ~GOrgueEventHandler()
	{
	}

	virtual void ProcessMidi(const GOrgueMidiEvent& event) = 0;
	virtual void HandleKey(int key) = 0;
};

#endif
