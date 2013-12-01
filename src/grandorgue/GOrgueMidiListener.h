/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMIDILISTENER_H
#define GORGUEMIDILISTENER_H

class GOrgueMidi;
class GOrgueMidiEvent;

class GOrgueMidiCallback
{
public:
	virtual ~GOrgueMidiCallback()
	{
	}

	
	virtual void OnMidiEvent(const GOrgueMidiEvent& event) = 0;
};

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
