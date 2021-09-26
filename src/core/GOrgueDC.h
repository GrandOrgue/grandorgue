/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDC_H
#define GORGUEDC_H

#include <wx/string.h>

class GOrgueBitmap;
class GOrgueFont;
class wxDC;
class wxColour;
class wxRect;

class GOrgueDC
{
private:
	wxDC* m_DC;
	double m_Scale;
	double m_FontScale;

	wxString WrapText(const wxString& string, unsigned width);

public:
	GOrgueDC(wxDC* dc, double scale, double fontScale);

	void DrawBitmap(GOrgueBitmap& bitmap, const wxRect& target);
	void DrawText(const wxString& text, const wxRect& rect, const wxColour& color, GOrgueFont& font, unsigned text_width, bool top = false);

	wxRect ScaleRect(const wxRect& rect);
};

#endif
