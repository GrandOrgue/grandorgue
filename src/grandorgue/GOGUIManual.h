/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GOGUIMANUAL_H
#define GOGUIMANUAL_H

#include "GOGUIControl.h"

class GOrgueManual;

class GOGUIManual : public GOGUIControl
{
private:
	GOrgueManual* m_manual;
	unsigned m_ManualNumber;
	bool m_key_colour_inverted;
	bool m_key_colour_wooden;

	void GetKeyDimensions(unsigned key_midi_nb, int &x, int &cx, int &cy, int &z);
	wxRegion GetKeyRegion(unsigned key_nb);
	void DrawKey(wxDC* dc, unsigned key_nb);

public:
	GOGUIManual(GOGUIPanel* panel, GOrgueManual* manual, unsigned manual_number);

	void Load(IniFileConfig& cfg, wxString group);

	void Draw(wxDC* dc);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
};

#endif
