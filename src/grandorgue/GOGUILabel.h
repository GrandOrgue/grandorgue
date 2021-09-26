/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUILABEL_H
#define GOGUILABEL_H

#include "GOGUIControl.h"
#include "GOrgueBitmap.h"
#include "GOrgueFont.h"
#include <wx/colour.h>

class GOrgueLabel;

class GOGUILabel : public GOGUIControl
{
private:
	int m_DispXpos;
	int m_DispYpos;
	GOrgueLabel* m_Label;
	GOrgueBitmap m_Bitmap;
	unsigned m_FontSize;
	wxString m_FontName;
	GOrgueFont m_Font;
	wxString m_Text;
	wxColour m_TextColor;
	wxRect m_TextRect;
	unsigned m_TextWidth;
	unsigned m_TileOffsetX;
	unsigned m_TileOffsetY;

public:
	GOGUILabel(GOGUIPanel* panel, GOrgueLabel* label);
	void Init(GOrgueConfigReader& cfg, wxString group, unsigned x_pos = 0, unsigned y_pos = 0, wxString name = wxT(""), unsigned imageno = 1);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
};

#endif
