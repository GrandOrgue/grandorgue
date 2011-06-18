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

#ifndef GORGUEMANUAL_H
#define GORGUEMANUAL_H

#include <wx/wx.h>
#include <vector>

class GOrgueCoupler;
class GOrgueDisplayMetrics;
class GOrgueDivisional;
class GOrgueStop;
class GOrgueTremulant;
class IniFileConfig;

class GOrgueManual 
{

private:

	int m_manual_number;
	GOrgueDisplayMetrics* m_display_metrics;
	wxInt16 m_midi[85];
	int m_first_accessible_logical_key_nb;
	unsigned m_nb_logical_keys;
	int m_first_accessible_key_midi_note_nb;
	unsigned m_nb_accessible_keys;

	int m_midi_input_number;

	unsigned m_nb_stops;
	unsigned m_nb_couplers;
	unsigned m_nb_divisionals;
	unsigned m_nb_tremulants;

	wxInt16 m_tremulant_ids[10];

	wxString m_name;

	std::vector<GOrgueStop*> m_stops;
	GOrgueCoupler* m_couplers;
	GOrgueDivisional* m_divisionals;
	bool m_displayed : 1;
	bool m_key_colour_inverted : 1;
	bool m_key_colour_wooden : 1;

	void GetKeyDimensions(const int key_midi_nb, int &x, int &cx, int &cy, int &z);
	wxRegion GetKeyRegion(unsigned key_nb);

public:

	GOrgueManual();
	void Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics, int manualNumber);
	void Set(int note, bool on, bool pretend = false, int depth = 0, GOrgueCoupler* prev = 0);
	void MIDI(void);
	~GOrgueManual(void);

	int GetNumberOfAccessibleKeys();
	int GetFirstAccessibleKeyMIDINoteNumber();
	int GetMIDIInputNumber();
	int GetLogicalKeyCount();
	void AllNotesOff();
	void MIDIPretend(bool on);
	bool IsKeyDown(int midiNoteNumber);

	int GetStopCount();
	GOrgueStop* GetStop(unsigned index);
	int GetCouplerCount();
	GOrgueCoupler* GetCoupler(unsigned index);
	int GetDivisionalCount();
	GOrgueDivisional* GetDivisional(unsigned index);
	int GetTremulantCount();
	GOrgueTremulant* GetTremulant(unsigned index);

	void DrawKey(wxDC& dc, unsigned key_nb);
	void Draw(wxDC& dc);
	bool IsDisplayed();

};

#endif
