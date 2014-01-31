/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

	wxString WrapText(const wxString& string, unsigned width);

public:
	GOrgueDC(wxDC* dc, double m_Scale);

	void DrawBitmap(GOrgueBitmap& bitmap, int x, int y);
	void TileBitmap(GOrgueBitmap& bitmap, const wxRect& target, int xo, int yo);
	void DrawText(const wxString& text, const wxRect& rect, const wxColour& color, GOrgueFont& font, unsigned text_width, bool top = false);

	wxRect ScaleRect(const wxRect& rect);
};

#endif
