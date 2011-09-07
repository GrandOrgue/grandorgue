/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GORGUEENCLOSURE_H_
#define GORGUEENCLOSURE_H_

#include "GOrgueDrawable.h"
#include "GOrgueMidiReceiver.h"
#include <wx/wx.h>

class GOrgueDisplayMetrics;
class GOrgueMidiEvent;
class GrandOrgueFile;
class IniFileConfig;

class GOrgueEnclosure : public GOrgueDrawable
{
private:
	wxString m_group;
	GOrgueMidiReceiver m_midi;
	GrandOrgueFile* m_organfile;
	unsigned m_enclosure_nb;
	int AmpMinimumLevel;
	int MIDIInputNumber;
	int MIDIValue;
	wxString Name;
	GOrgueDisplayMetrics* DisplayMetrics;

public:

	GOrgueEnclosure(GrandOrgueFile* organfile);
	bool Draw(int xx, int yy, wxDC* dc = 0, wxDC* dc2 = 0);
	void Load(IniFileConfig& cfg, const unsigned enclosure_nb, GOrgueDisplayMetrics* displayMetrics);
	void Save(IniFileConfig& cfg, bool prefix);
	void Set(int n);
	void ProcessMidi(const GOrgueMidiEvent& event);
	void MIDI(void);
	int GetMIDIInputNumber();
	float GetAttenuation();

	void Scroll(bool scroll_up);

	void DrawLabel(wxDC& dc);

	bool IsEnclosure(const unsigned nb) const;

};

#endif /* GORGUEENCLOSURE_H_ */
