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

#ifndef GORGUEMETRONOME_H
#define GORGUEMETRONOME_H

#include "GOrgueElementCreator.h"
#include "GOrgueLabel.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"
#include "GOrgueTimerCallback.h"

class GOrgueMidiEvent;
class GOrgueRank;
class GrandOrgueFile;

class GOrgueMetronome : private GOrgueTimerCallback, private GOrguePlaybackStateHandler,
	private GOrgueSaveableObject, public GOrgueElementCreator
{
private:
	GrandOrgueFile* m_organfile;
	unsigned m_BPM;
	unsigned m_MeasureLength;
	unsigned m_Pos;
	bool m_Running;
	GOrgueLabel m_BPMDisplay;
	GOrgueLabel m_MeasureDisplay;
	GOrgueRank* m_rank;
	unsigned m_StopID;

	static const struct ElementListEntry m_element_types[];
	const struct ElementListEntry* GetButtonList();

	void HandleTimer();

	void ButtonChanged(int id);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

	void Save(GOrgueConfigWriter& cfg);

	void StartTimer();
	void StopTimer();
	void UpdateState();
	void UpdateBPM(int val);
	void UpdateMeasure(int val);

public:
	GOrgueMetronome(GrandOrgueFile* organfile);
	virtual ~GOrgueMetronome();

	void Load(GOrgueConfigReader& cfg);

	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel);
};

#endif
