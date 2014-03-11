/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMIDIPLAYER_H
#define GORGUEMIDIPLAYER_H

#include "GOrgueTime.h"
#include <wx/string.h>
#include <wx/timer.h>
#include <vector>

class GOrgueMidi;
class GOrgueMidiEvent;
class GOrgueMidiFileReader;

class GOrgueMidiPlayer : private wxTimer
{
private:
	GOrgueMidi& m_midi;
	std::vector<GOrgueMidiEvent> m_Events;
	unsigned m_Pos;
	GOTime m_Last;
	float m_Speed;
	bool m_IsPlaying;
	unsigned m_DeviceID;

	void ReadFileContent(GOrgueMidiFileReader& reader, std::vector<GOrgueMidiEvent>& events);
	void SetupManual(unsigned channel, wxString ID);
	void Notify();

public:
	GOrgueMidiPlayer(GOrgueMidi& midi);
	~GOrgueMidiPlayer();

	void Clear();
	void Load(wxString filename, unsigned manuals, bool pedal);
	bool IsLoaded();

	void Play();
	void StopPlayer();
	bool IsPlaying();
};

#endif
