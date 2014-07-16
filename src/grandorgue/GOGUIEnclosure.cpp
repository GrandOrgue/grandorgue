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

#include "GOGUIEnclosure.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIMouseState.h"
#include "GOGUIPanel.h"
#include "GOrgueConfigReader.h"
#include "GOrgueDC.h"
#include "GOrgueEnclosure.h"
#include <wx/intl.h>

GOGUIEnclosure::GOGUIEnclosure(GOGUIPanel* panel, GOrgueEnclosure* control):
	GOGUIControl(panel, control),
	m_enclosure(control),
	m_FontSize(0),
	m_FontName(),
	m_TextColor(0,0,0),
	m_Text(),
	m_TextWidth(0),
	m_MouseAxisStart(0),
	m_MouseAxisEnd(0),
	m_TileOffsetX(0),
	m_TileOffsetY(0),
	m_Bitmaps(0)
{
	m_layout->RegisterEnclosure(this);
}

void GOGUIEnclosure::Init(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Init(cfg, group);

	unsigned bitmap_count = 16;

	for(unsigned i = 1; i <= bitmap_count; i++)
	{
		wxString bitmap = wxString::Format(wxT("GO:enclosure%c%02d"), wxT('B'), i - 1);
		wxString mask = wxEmptyString;
		m_Bitmaps.push_back(m_panel->LoadBitmap(bitmap, mask));
	}

	for(unsigned i = 1; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[0].GetWidth() != m_Bitmaps[i].GetWidth() || 
		    m_Bitmaps[0].GetHeight() != m_Bitmaps[i].GetHeight())
			throw wxString::Format(_("bitmap size does not match for '%s'"), group.c_str());

	int x, y, w, h;

	x = -1;
	y = -1;
	w = m_Bitmaps[0].GetWidth();
	h = m_Bitmaps[0].GetHeight();
	m_BoundingRect = wxRect(x, y, w, h);

	m_TileOffsetX = 0;
	m_TileOffsetY = 0;

	m_TextColor = wxColour(0xFF, 0xFF, 0xFF);
	m_FontSize = 7;
	m_FontName = wxT("");
	m_Text = m_enclosure->GetName();

	x = 0;
	y = 0;
	w = m_BoundingRect.GetWidth() - x;
	h = m_BoundingRect.GetHeight() - y;
	m_TextRect = wxRect(x, y, w, h);
	m_TextWidth = m_TextRect.GetWidth();

	x = 0;
	y = 13;
	w = m_BoundingRect.GetWidth() - x;
	h = m_BoundingRect.GetHeight() - y - 3;
	m_MouseRect = wxRect(x, y, w, h);

	m_MouseAxisStart = m_MouseRect.GetHeight() / 3;
	m_MouseAxisEnd = m_MouseRect.GetHeight() / 3 * 2;

	m_Font.SetName(m_FontName);
	m_Font.SetPoints(m_FontSize);
}

void GOGUIEnclosure::Load(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);

	unsigned bitmap_count = cfg.ReadInteger(ODFSetting, group, wxT("BitmapCount"), 1, 127, false, 16);
	wxChar style = wxT('A') + cfg.ReadInteger(ODFSetting, group, wxT("EnclosureStyle"), 1, 4, false, 1) - 1;

	for(unsigned i = 1; i <= bitmap_count; i++)
	{
		wxString bitmap = cfg.ReadStringTrim(ODFSetting, group, wxString::Format(wxT("Bitmap%03d"), i), false, wxString::Format(wxT("GO:enclosure%c%02d"), style, i - 1));
		wxString mask = cfg.ReadStringTrim(ODFSetting, group, wxString::Format(wxT("Mask%03d"), i), false, wxEmptyString);
		m_Bitmaps.push_back(m_panel->LoadBitmap(bitmap, mask));
	}

	for(unsigned i = 1; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[0].GetWidth() != m_Bitmaps[i].GetWidth() || 
		    m_Bitmaps[0].GetHeight() != m_Bitmaps[i].GetHeight())
			throw wxString::Format(_("bitmap size does not match for '%s'"), group.c_str());

	int x, y, w, h;

	x = cfg.ReadInteger(ODFSetting, group, wxT("PositionX"), 0, m_metrics->GetScreenWidth(), false, -1);
	y = cfg.ReadInteger(ODFSetting, group, wxT("PositionY"), 0, m_metrics->GetScreenHeight(), false, -1);
	w = cfg.ReadInteger(ODFSetting, group, wxT("Width"), 1, m_metrics->GetScreenWidth(), false, m_Bitmaps[0].GetWidth());
	h = cfg.ReadInteger(ODFSetting, group, wxT("Height"), 1, m_metrics->GetScreenHeight(), false, m_Bitmaps[0].GetHeight());
	m_BoundingRect = wxRect(x, y, w, h);

	m_TileOffsetX = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetX"), 0, m_Bitmaps[0].GetWidth() - 1, false, 0);
	m_TileOffsetY = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetY"), 0, m_Bitmaps[0].GetHeight() - 1, false, 0);

	m_TextColor = cfg.ReadColor(ODFSetting, group, wxT("DispLabelColour"), false, wxT("White"));
	m_FontSize = cfg.ReadFontSize(ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("7"));
	m_FontName = cfg.ReadStringTrim(ODFSetting, group, wxT("DispLabelFontName"), false, wxT(""));
	m_Text = cfg.ReadString(ODFSetting, group, wxT("DispLabelText"), false, m_enclosure->GetName());

	x = cfg.ReadInteger(ODFSetting, group, wxT("TextRectLeft"), 0, m_BoundingRect.GetWidth() - 1, false, 0);
	y = cfg.ReadInteger(ODFSetting, group, wxT("TextRectTop"), 0, m_BoundingRect.GetHeight() - 1, false, 0);
	w = cfg.ReadInteger(ODFSetting, group, wxT("TextRectWidth"), 1, m_BoundingRect.GetWidth() - x, false, m_BoundingRect.GetWidth() - x);
	h = cfg.ReadInteger(ODFSetting, group, wxT("TextRectHeight"), 1, m_BoundingRect.GetHeight() - y, false, m_BoundingRect.GetHeight() - y);
	m_TextRect = wxRect(x, y, w, h);
	m_TextWidth = cfg.ReadInteger(ODFSetting, group, wxT("TextBreakWidth"), 0, m_TextRect.GetWidth(), false, m_TextRect.GetWidth());

	x = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectLeft"), 0, m_BoundingRect.GetWidth() - 1, false, 0);
	y = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectTop"), 0, m_BoundingRect.GetHeight() - 1, false, 13);
	w = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectWidth"), 1, m_BoundingRect.GetWidth() - x, false, m_BoundingRect.GetWidth() - x);
	h = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectHeight"), 1, m_BoundingRect.GetHeight() - y, false, m_BoundingRect.GetHeight() - y - 3);
	m_MouseRect = wxRect(x, y, w, h);

	m_MouseAxisStart = cfg.ReadInteger(ODFSetting, group, wxT("MouseAxisStart"), 0, m_MouseRect.GetHeight(), false, m_MouseRect.GetHeight() / 3);
	m_MouseAxisEnd = cfg.ReadInteger(ODFSetting, group, wxT("MouseAxisEnd"), m_MouseAxisStart, m_MouseRect.GetHeight(), false, m_MouseRect.GetHeight() / 3 * 2);

	m_Font.SetName(m_FontName);
	m_Font.SetPoints(m_FontSize);
}

void GOGUIEnclosure::Layout()
{
	if (m_BoundingRect.GetX() == -1)
		m_BoundingRect.SetX(m_layout->GetEnclosureX(this));
	if (m_BoundingRect.GetY() == -1)
		m_BoundingRect.SetY(m_layout->GetEnclosureY());

	m_TextRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
	m_MouseRect.Offset(m_BoundingRect.GetX(), m_BoundingRect.GetY());
}

void GOGUIEnclosure::Draw(GOrgueDC& dc)
{
	GOrgueBitmap& bmp = m_Bitmaps[((m_Bitmaps.size() - 1) * m_enclosure->GetValue()) / 127];
	dc.TileBitmap(bmp, m_BoundingRect, m_TileOffsetX, m_TileOffsetY);

	if (m_TextWidth)
		dc.DrawText(m_Text, m_TextRect, m_TextColor, m_Font, m_TextWidth, true);

	GOGUIControl::Draw(dc);
}


bool GOGUIEnclosure::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	if (!m_BoundingRect.Contains(x, y))
		return false;
	if (right)
	{
		m_enclosure->ShowConfigDialog();
		return true;
	}
	else
	{
		unsigned value;
		if (!m_MouseRect.Contains(x, y))
			return false;
		y -= m_MouseRect.GetY();
		if (y <= m_MouseAxisStart)
			value = (127 * y / m_MouseAxisStart);
		else if (y >= m_MouseAxisEnd)
			value = (m_MouseRect.GetHeight() - y) * 127 / (m_MouseRect.GetHeight() - m_MouseAxisEnd);
		else
			value = 127;

		if (state.GetControl() == this && state.GetIndex() == value)
			return true;
		state.SetControl(this);
		state.SetIndex(value);

		m_enclosure->Set(value);
		return true;
	}
}

bool GOGUIEnclosure::HandleMouseScroll(int x, int y, int amount)
{
	if (!m_BoundingRect.Contains(x, y) || !amount)
		return false;
	m_enclosure->Scroll(amount > 0);
	return true;
}
