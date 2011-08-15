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
#include "ptrvector.h"

class wxProgressDialog;

class GOrgueCoupler;
class GOrgueDisplayMetrics;
class GOrgueDivisional;
class GOrgueStop;
class GOrgueTremulant;
class IniFileConfig;

class GOrgueManual 
{

private:

	/* Keyboard state */
	std::vector<bool> m_KeyPressed;
	/* Internal state affected by couplers */
	std::vector<unsigned> m_KeyState;
	int m_manual_number;
	GOrgueDisplayMetrics* m_display_metrics;
	unsigned m_first_accessible_logical_key_nb;
	unsigned m_nb_logical_keys;
	unsigned m_first_accessible_key_midi_note_nb;
	unsigned m_nb_accessible_keys;
	unsigned m_UnisonOff;

	int m_midi_input_number;

	std::vector<unsigned> m_tremulant_ids;

	wxString m_name;

	ptr_vector<GOrgueStop> m_stops;
	ptr_vector<GOrgueCoupler> m_couplers;
	ptr_vector<GOrgueDivisional> m_divisionals;
	bool m_displayed : 1;
	bool m_key_colour_inverted : 1;
	bool m_key_colour_wooden : 1;

	void GetKeyDimensions(const int key_midi_nb, int &x, int &cx, int &cy, int &z);
	wxRegion GetKeyRegion(unsigned key_nb);

public:

	GOrgueManual();
	void Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics, int manualNumber);
	void Save(IniFileConfig& cfg, bool prefix, wxString group);
	void SetKey(unsigned note, int on, GOrgueCoupler* prev);
	void Set(unsigned note, bool on);
	void SetUnisonOff(bool on);
	void Abort();
	void PreparePlayback();
	void MIDI(void);
	~GOrgueManual(void);

	unsigned GetNumberOfAccessibleKeys();
	unsigned GetFirstAccessibleKeyMIDINoteNumber();
	int GetFirstLogicalKeyMIDINoteNumber();
	int GetMIDIInputNumber();
	unsigned GetLogicalKeyCount();
	void AllNotesOff();
	bool IsKeyDown(unsigned midiNoteNumber);

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
