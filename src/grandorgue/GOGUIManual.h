/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#ifndef GOGUIMANUAL_H
#define GOGUIMANUAL_H

#include <vector>
#include "GOGUIControl.h"

class GOrgueManual;

class GOGUIManual : public GOGUIControl
{
private:
	typedef struct {
		unsigned MidiNumber;
		bool IsSharp;
		wxRect Rect;
		wxBitmap* OnBitmap;
		wxBitmap* OffBitmap;
	} KeyInfo;

	GOrgueManual* m_manual;
	unsigned m_ManualNumber;
	std::vector<KeyInfo> m_Keys;

public:
	GOGUIManual(GOGUIPanel* panel, GOrgueManual* manual, unsigned manual_number);

	void Load(IniFileConfig& cfg, wxString group);

	void Draw(wxDC* dc);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
};

#endif
