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

#ifndef GOGUIENCLOSURE_H
#define GOGUIENCLOSURE_H

#include <vector>
#include "GOGUIControl.h"

class GOrgueEnclosure;

class GOGUIEnclosure : public GOGUIControl
{
private:
	GOrgueEnclosure* m_enclosure;
	unsigned m_enclosure_nb;
	unsigned m_FontSize;
	wxString m_FontName;
	wxColor m_TextColor;
	wxString m_Text;
	wxRect m_TextRect;
	unsigned m_TextWidth;
	wxRect m_MouseRect;
	int m_MouseAxisStart;
	int m_MouseAxisEnd;
	unsigned m_TileOffsetX;
	unsigned m_TileOffsetY;
	std::vector<wxBitmap*> m_Bitmaps;
	
public:
	GOGUIEnclosure(GOGUIPanel* panel, GOrgueEnclosure* control, unsigned enclosure_nb);

	void Load(GOrgueConfigReader& cfg, wxString group);

	void Draw(wxDC* dc);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	void HandleMouseScroll(int x, int y, int amount);

	bool IsEnclosure(const unsigned nb) const;
};

#endif
