/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUIHW1BACKGROUND_H
#define GOGUIHW1BACKGROUND_H

#include "GOGUIControl.h"

#include "GOrgueBitmap.h"
#include <vector>

class GOGUIHW1Background : public GOGUIControl
{
	class GOBackgroundImage
	{
	public:

	wxRect rect;
	GOrgueBitmap bmp;

	GOBackgroundImage(wxRect Rect, const GOrgueBitmap& Bmp) :
		rect(Rect),
		bmp(Bmp)
		{
		}
	};
private:
	std::vector<GOBackgroundImage> m_Images;

public:
	GOGUIHW1Background(GOGUIPanel* panel);

	void Init(GOrgueConfigReader& cfg, wxString group);
	void Layout();

	void PrepareDraw(double scale, GOrgueBitmap* background);
	void Draw(GOrgueDC& dc);
};

#endif
