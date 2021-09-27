/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDILISTENER_H
#define GORGUEMIDILISTENER_H

class GOrgueMidi;
class GOrgueMidiCallback;
class GOrgueMidiEvent;

class GOrgueMidiListener
{
	GOrgueMidiCallback* m_Callback;
	GOrgueMidi* m_midi;

public:
	GOrgueMidiListener();
	virtual ~GOrgueMidiListener();

	void SetCallback(GOrgueMidiCallback* callback);
	void Register(GOrgueMidi* midi);
	void Unregister();

	void Send(const GOrgueMidiEvent& event);
};

#endif
