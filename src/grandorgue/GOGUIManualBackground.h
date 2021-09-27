/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIMANUALBACKGROUND_H
#define GOGUIMANUALBACKGROUND_H

#include "GOGUIControl.h"

#include "GOrgueBitmap.h"

class GOGUIManualBackground : public GOGUIControl
{
private:
	unsigned m_ManualNumber;
	wxRect m_VRect;
	GOrgueBitmap m_VBackground;
	wxRect m_HRect;
	GOrgueBitmap m_HBackground;

public:
	GOGUIManualBackground(GOGUIPanel* panel, unsigned manual_number);

	void Init(GOrgueConfigReader& cfg, wxString group);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
};

#endif
