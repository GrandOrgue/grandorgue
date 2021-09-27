/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIENCLOSURE_H
#define GOGUIENCLOSURE_H

#include "GOGUIControl.h"
#include "GOrgueBitmap.h"
#include "GOrgueFont.h"
#include <wx/colour.h>
#include <vector>

class GOrgueEnclosure;

class GOGUIEnclosure : public GOGUIControl
{
private:
	GOrgueEnclosure* m_enclosure;
	unsigned m_FontSize;
	wxString m_FontName;
	GOrgueFont m_Font;
	wxColor m_TextColor;
	wxString m_Text;
	wxRect m_TextRect;
	unsigned m_TextWidth;
	wxRect m_MouseRect;
	int m_MouseAxisStart;
	int m_MouseAxisEnd;
	unsigned m_TileOffsetX;
	unsigned m_TileOffsetY;
	std::vector<GOrgueBitmap> m_Bitmaps;
	
public:
	GOGUIEnclosure(GOGUIPanel* panel, GOrgueEnclosure* control);

	void Init(GOrgueConfigReader& cfg, wxString group);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
	bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
	bool HandleMouseScroll(int x, int y, int amount);
};

#endif
