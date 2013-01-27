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

#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrgueConfigReader.h"
#include "GOrgueLabel.h"

GOGUILabel::GOGUILabel(GOGUIPanel* panel, GOrgueLabel* label, unsigned x_pos, unsigned y_pos, wxString name) :
	GOGUIControl(panel, label),
	m_DispXpos(x_pos),
	m_DispYpos(y_pos),
	m_Label(label),
	m_Bitmap(0),
	m_FontSize(0),
	m_FontName(),
	m_Text(name),
	m_TextColor(0,0,0),
	m_TextRect(),
	m_TextWidth(0),
	m_TileOffsetX(0),
	m_TileOffsetY(0)
{
}

void GOGUILabel::Load(GOrgueConfigReader& cfg, wxString group)
{
	bool FreeXPlacement = cfg.ReadBoolean(ODFSetting, group, wxT("FreeXPlacement"), false, true);
	bool FreeYPlacement = cfg.ReadBoolean(ODFSetting, group, wxT("FreeYPlacement"), false, true);

	if (!FreeXPlacement)
	{
		int DispDrawstopCol = cfg.ReadInteger(ODFSetting, group, wxT("DispDrawstopCol"), 1, m_metrics->NumberOfDrawstopColsToDisplay(), true, 1);
		bool DispSpanDrawstopColToRight = cfg.ReadBoolean(ODFSetting, group, wxT("DispSpanDrawstopColToRight"), true, false);

		int i = m_metrics->NumberOfDrawstopColsToDisplay() >> 1;
		if (DispDrawstopCol <= i)
			m_DispXpos = m_metrics->GetJambLeftX()  + (DispDrawstopCol - 1) * 78 + 1;
		else
			m_DispXpos = m_metrics->GetJambRightX() + (DispDrawstopCol - 1 - i) * 78 + 1;
		if (DispSpanDrawstopColToRight)
			m_DispXpos += 39;
	}
	else
	{
		m_DispXpos = cfg.ReadInteger(ODFSetting, group, wxT("DispXpos"), 0, m_metrics->GetScreenWidth(), false, m_DispXpos);
	}

	if (!FreeYPlacement)
	{
		bool DispAtTopOfDrawstopCol = cfg.ReadBoolean(ODFSetting, group, wxT("DispAtTopOfDrawstopCol"), true, false);

		m_DispYpos = m_metrics->GetJambLeftRightY() + 1;
		if (!DispAtTopOfDrawstopCol)
			m_DispYpos += m_metrics->GetJambLeftRightHeight() - 32;
	}
	else
	{
		m_DispYpos = cfg.ReadInteger(ODFSetting, group, wxT("DispYpos"), 0, m_metrics->GetScreenHeight(), false, m_DispYpos);
	}

	m_TextColor = cfg.ReadColor(ODFSetting, group, wxT("DispLabelColour"), false, wxT("BLACK"));
	m_FontSize = cfg.ReadFontSize(ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("normal"));
	m_FontName = cfg.ReadString(ODFSetting, group, wxT("DispLabelFontName"), 255, false, wxT(""));
	m_Text = cfg.ReadString(ODFSetting, group, wxT("Name"), 64, false, m_Text);

	unsigned DispImageNum = cfg.ReadInteger(ODFSetting, group, wxT("DispImageNum"), 1, 1, false, 1);

	wxString image_file = cfg.ReadString(ODFSetting, group, wxT("Image"), 255, false, wxString::Format(wxT("GO:label%02d"), DispImageNum));
	wxString image_mask_file = cfg.ReadString(ODFSetting, group, wxT("Mask"), 255, false, wxEmptyString);

	m_Bitmap = m_panel->LoadBitmap(image_file, image_mask_file);

	int x, y, w, h;
	x = cfg.ReadInteger(ODFSetting, group, wxT("PositionX"), 0, m_metrics->GetScreenWidth(), false, m_DispXpos);
	y = cfg.ReadInteger(ODFSetting, group, wxT("PositionY"), 0, m_metrics->GetScreenHeight(), false, m_DispYpos);
	w = cfg.ReadInteger(ODFSetting, group, wxT("Width"), 1, m_metrics->GetScreenWidth(), false, m_Bitmap->GetWidth());
	h = cfg.ReadInteger(ODFSetting, group, wxT("Height"), 1, m_metrics->GetScreenHeight(), false, m_Bitmap->GetHeight());
	m_BoundingRect = wxRect(x, y, w, h);

	m_TileOffsetX = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetX"), 0, m_Bitmap->GetWidth() - 1, false, 0);
	m_TileOffsetY = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetY"), 0, m_Bitmap->GetHeight() - 1, false, 0);

	x = cfg.ReadInteger(ODFSetting, group, wxT("TextRectLeft"), 0, m_BoundingRect.GetWidth() - 1, false, 1);
	y = cfg.ReadInteger(ODFSetting, group, wxT("TextRectTop"), 0, m_BoundingRect.GetHeight() - 1, false, 1);
	w = cfg.ReadInteger(ODFSetting, group, wxT("TextRectWidth"), 1, m_BoundingRect.GetWidth() - x, false, m_BoundingRect.GetWidth() - x);
	h = cfg.ReadInteger(ODFSetting, group, wxT("TextRectHeight"), 1, m_BoundingRect.GetHeight() - y, false, m_BoundingRect.GetHeight() - y);
	m_TextRect = wxRect(x + m_BoundingRect.GetX(), y + m_BoundingRect.GetY(), w, h);
	m_TextWidth = cfg.ReadInteger(ODFSetting, group, wxT("TextBreakWidth"), 0, m_TextRect.GetWidth(), false, m_TextRect.GetWidth());
}

void GOGUILabel::Draw(wxDC* dc)
{
	if (m_Label)
		m_Text = m_Label->GetName();

	m_panel->TileBitmap(dc, m_Bitmap, m_BoundingRect, m_TileOffsetX, m_TileOffsetY);
	if (m_TextWidth)
	{
		dc->SetTextForeground(m_TextColor);
		wxFont font = m_metrics->GetGroupLabelFont();
		if (m_FontName != wxEmptyString)
		{
			wxFont new_font = font;
			if (new_font.SetFaceName(m_FontName))
				font = new_font;
		}
		font.SetPointSize(m_FontSize);
		dc->SetFont(font);
		dc->DrawLabel(m_panel->WrapText(dc, m_Text, m_TextWidth), m_TextRect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	}
	GOGUIControl::Draw(dc);
}
