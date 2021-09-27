/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIBUTTON_H
#define GOGUIBUTTON_H

#include "GOGUIControl.h"
#include "GOrgueBitmap.h"
#include "GOrgueFont.h"
#include <wx/colour.h>

class GOrgueButton;

class GOGUIButton : public GOGUIControl
{
protected:
	bool m_IsPiston;
	bool m_DispKeyLabelOnLeft;
	GOrgueButton* m_Button;
	wxRect m_MouseRect;
	unsigned m_Radius;
	GOrgueBitmap m_OnBitmap;
	GOrgueBitmap m_OffBitmap;
	unsigned m_FontSize;
	wxString m_FontName;
	GOrgueFont m_Font;
	wxColor m_TextColor;
	wxString m_Text;
	wxRect m_TextRect;
	unsigned m_TextWidth;
	unsigned m_DispCol;
	unsigned m_DispRow;
	unsigned m_TileOffsetX;
	unsigned m_TileOffsetY;

public:
	GOGUIButton(GOGUIPanel* panel, GOrgueButton* control, bool is_piston = false);

	void Init(GOrgueConfigReader& cfg, wxString group, unsigned x_pos, unsigned y_pos, unsigned image = 0);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
};

#endif
