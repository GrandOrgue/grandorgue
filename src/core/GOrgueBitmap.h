/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEBITMAP_H
#define GORGUEBITMAP_H

#include <wx/bitmap.h>

class wxImage;

class GOrgueBitmap
{
private:
	wxImage* m_img;
	wxBitmap m_bmp;
	double m_Scale;
	int m_ResultWidth;
	int m_ResultHeight;
	unsigned m_ResultXOffset;
	unsigned m_ResultYOffset;

	void ScaleBMP(wxImage& img, double scale, const wxRect& rect, GOrgueBitmap* background);

public:
	GOrgueBitmap();
	GOrgueBitmap(wxImage* img);

	void PrepareBitmap(double scale, const wxRect& rect, GOrgueBitmap* background);
	void PrepareTileBitmap(double scale, const wxRect& rect, unsigned xo, unsigned yo, GOrgueBitmap* background);

	unsigned GetWidth();
	unsigned GetHeight();

	const wxBitmap& GetBitmap();
};

#endif
