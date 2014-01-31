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

#ifndef GORGUEFONT_H
#define GORGUEFONT_H

#include <wx/font.h>
#include <wx/string.h>

class GOrgueFont
{
private:
	wxFont m_Font;
	wxFont m_ScaledFont;
	wxString m_Name;
	unsigned m_Points;
	double m_Scale;

public:
	GOrgueFont();

	void SetName(const wxString& name);
	void SetPoints(unsigned points);
	wxFont GetFont(double scale);
};

#endif
