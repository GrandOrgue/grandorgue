/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIMANUAL_H
#define GOGUIMANUAL_H

#include "GOGUIControl.h"
#include "GOrgueBitmap.h"
#include <vector>

class GOrgueManual;

class GOGUIManual : public GOGUIControl
{
private:
	typedef struct {
		unsigned MidiNumber;
		bool IsSharp;
		wxRect Rect;
		wxRect MouseRect;
		GOrgueBitmap OnBitmap;
		GOrgueBitmap OffBitmap;
	} KeyInfo;

	GOrgueManual* m_manual;
	unsigned m_ManualNumber;
	std::vector<KeyInfo> m_Keys;

public:
	GOGUIManual(GOGUIPanel* panel, GOrgueManual* manual, unsigned manual_number);

	bool IsSharp(unsigned key);
	unsigned GetKeyCount();

	void Init(GOrgueConfigReader& cfg, wxString group);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
	bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
};

#endif
