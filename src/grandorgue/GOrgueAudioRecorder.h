/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEAUDIORECORDER_H
#define GORGUEAUDIORECORDER_H

#include "GOrgueElementCreator.h"
#include "GOrgueLabel.h"
#include "GOrgueTimerCallback.h"
#include <wx/string.h>

class GrandOrgueFile;
class GOSoundRecorder;

class GOrgueAudioRecorder : public GOrgueElementCreator, private GOrgueTimerCallback
{
private:
	GrandOrgueFile* m_organfile;
	GOSoundRecorder* m_recorder;
	GOrgueLabel m_RecordingTime;
	unsigned m_RecordSeconds;
	wxString m_Filename;
	bool m_DoRename;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void ButtonChanged(int id);

	void UpdateDisplay();
	void HandleTimer();

public:
	GOrgueAudioRecorder(GrandOrgueFile* organfile);
	~GOrgueAudioRecorder();

	void SetAudioRecorder(GOSoundRecorder* recorder);

	void StartRecording(bool rename);
	bool IsRecording();
	void StopRecording();

	void Load(GOrgueConfigReader& cfg);
	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
