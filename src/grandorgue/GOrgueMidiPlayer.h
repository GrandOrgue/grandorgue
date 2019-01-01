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

#ifndef GORGUEMIDIPLAYER_H
#define GORGUEMIDIPLAYER_H

#include "GOrgueLabel.h"
#include "GOrgueElementCreator.h"
#include "GOrgueMidiPlayerContent.h"
#include "GOrgueTime.h"
#include "GOrgueTimerCallback.h"
#include <wx/string.h>
#include <wx/timer.h>
#include <vector>

class GOrgueMidiEvent;
class GOrgueMidiFileReader;
class GrandOrgueFile;

class GOrgueMidiPlayer : public GOrgueElementCreator, private GOrgueTimerCallback
{
private:
	GrandOrgueFile* m_organfile;
	GOrgueMidiPlayerContent m_content;
	GOrgueLabel m_PlayingTime;
	GOTime m_Start;
	unsigned m_PlayingSeconds;
	float m_Speed;
	bool m_IsPlaying;
	bool m_Pause;
	unsigned m_DeviceID;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void ButtonChanged(int id);

	void UpdateDisplay();
	void HandleTimer();

public:
	GOrgueMidiPlayer(GrandOrgueFile* organfile);
	~GOrgueMidiPlayer();

	void Clear();
	void LoadFile(const wxString& filename, unsigned manuals, bool pedal);
	bool IsLoaded();

	void Play();
	void Pause();
	void StopPlaying();
	bool IsPlaying();

	void Load(GOrgueConfigReader& cfg);
	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
