/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueDC.h"

#include "GOrgueBitmap.h"
#include "GOrgueFont.h"
#include <wx/dc.h>

GOrgueDC::GOrgueDC(wxDC* dc, double scale) :
	m_DC(dc),
	m_Scale(scale)
{
}

wxRect GOrgueDC::ScaleRect(const wxRect& rect)
{
	return wxRect(rect.GetX() * m_Scale + 0.5, rect.GetY() * m_Scale + 0.5, rect.GetWidth() * m_Scale + 0.5, rect.GetHeight() * m_Scale + 0.5);
}

void GOrgueDC::DrawBitmap(GOrgueBitmap& bitmap, int x, int y)
{
	m_DC->DrawBitmap(bitmap.GetData(m_Scale), x * m_Scale + 0.5, y * m_Scale + 0.5, true);
}

void GOrgueDC::TileBitmap(GOrgueBitmap& bitmap, const wxRect& tgt, int xo, int yo)
{
	wxRect target = ScaleRect(tgt);
	m_DC->SetClippingRegion(target);
	const wxBitmap& bmp = bitmap.GetData(m_Scale);
	for (int y = target.GetY() - yo * m_Scale; y < target.GetBottom(); y += bmp.GetHeight())
		for (int x = target.GetX() - xo * m_Scale; x < target.GetRight(); x += bmp.GetWidth())
			m_DC->DrawBitmap(bmp, x, y, true);
	m_DC->DestroyClippingRegion();
}

wxString GOrgueDC::WrapText(const wxString& string, unsigned width)
{
	wxString str, line, work;
	wxCoord cx, cy;

	/* string.Length() + 1 iterations */
	for(unsigned i = 0; i <= string.Length(); i++)
	{
		bool maybreak = false;
		if (string[i] == wxT(' ') || string[i] == wxT('\n'))
		{
			if (work.length() < 2)
				maybreak = false;
			else
				maybreak = true;
		}
		if (maybreak || i == string.Length())
		{
			if (!work.Length())
				continue;
			m_DC->GetTextExtent(line + wxT(' ') + work, &cx, &cy);
			if (cx > (int)width)
			{
				if (!str.Length())
					str = line;
				else
					str = str + wxT('\n') + line;
				line = wxT("");
			}

			if (!line.Length())
				line = work;
			else
				line = line + wxT(' ') + work;

			work = wxT("");
		}
		else
		{
			if (string[i] == wxT(' ') || string[i] == wxT('\n'))
			{
				if (work.Length() && work[work.Length()-1] != wxT(' '))
					work += wxT(' ');
			}	
			else
				work += string[i];
		}
	}
	if (!str.Length())
		str = line;
	else
		str = str + wxT('\n') + line;
	return str;
}

void GOrgueDC::DrawText(const wxString& text, const wxRect& rect, const wxColour& color, GOrgueFont& font, unsigned text_width, bool align_top)
{
	m_DC->SetTextForeground(color);
	m_DC->SetFont(font.GetFont(m_Scale));
	m_DC->DrawLabel(WrapText(text, m_Scale * text_width), ScaleRect(rect), (align_top ? 0 : wxALIGN_CENTER_VERTICAL) | wxALIGN_CENTER_HORIZONTAL);
}


