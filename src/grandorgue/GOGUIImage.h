/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIIMAGE_H
#define GOGUIIMAGE_H

#include "GOGUIControl.h"
#include "GOrgueBitmap.h"

class GOGUIImage : public GOGUIControl
{
protected:
	GOrgueBitmap m_Bitmap;
	unsigned m_TileOffsetX;
	unsigned m_TileOffsetY;

public:
	GOGUIImage(GOGUIPanel* panel);

	void Load(GOrgueConfigReader& cfg, wxString group);

	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
};

#endif
