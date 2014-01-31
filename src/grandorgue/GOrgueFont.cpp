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

#include "GOrgueFont.h"

#include <wx/gdicmn.h>

GOrgueFont::GOrgueFont() :
	m_Name(),
	m_Points(0),
	m_Scale(0)
{
	m_Font = *wxNORMAL_FONT;
}

void GOrgueFont::SetName(const wxString& name)
{
	if (name == wxEmptyString)
		return;
	wxFont new_font = m_Font;
	if (new_font.SetFaceName(name))
	{
		m_Font = new_font;
		m_Name = name;
		m_Scale = 0;
	}
}
void GOrgueFont::SetPoints(unsigned points)
{
	m_Font.SetPointSize(points);
	m_Points = points;
	m_Scale = 0;
}

wxFont GOrgueFont::GetFont(double scale)
{
	if (m_Scale != scale)
	{
		m_ScaledFont = m_Font;
		m_ScaledFont.SetPointSize(m_Points * scale);
		m_Scale = scale;
	}
	return m_ScaledFont;
}
