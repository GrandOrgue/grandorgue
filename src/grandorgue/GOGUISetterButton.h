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

#ifndef GOGUISETTERBUTTON_H
#define GOGUISETTERBUTTON_H

#include "GOGUIControl.h"

class GOrgueSetterButton;

class GOGUISetterButton : public GOGUIControl
{
private:
	GOrgueSetterButton* m_button;
	bool m_DispKeyLabelOnLeft;
	int m_DispLabelFontSize;
	wxColour m_DispLabelColour;
	int m_DispDrawstopRow;
	int m_DispDrawstopCol;
	int m_DispImageNum;
	bool m_Piston;
	
public:
	GOGUISetterButton(GOGUIPanel* panel, GOrgueSetterButton* control);

	void Init(IniFileConfig& cfg, unsigned x, unsigned y, wxString group, bool IsPiston = false);

	void Draw(wxDC* dc);
	void HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
};

#endif
