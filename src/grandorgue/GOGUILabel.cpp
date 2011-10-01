/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOGUILabel.h"
#include "GOGUIPanel.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrgueLabel.h"
#include "IniFileConfig.h"

GOGUILabel::GOGUILabel(GOGUIPanel* panel, GOrgueLabel* label) :
	GOGUIControl(panel, label),
	m_FreeXPlacement(false),
	m_FreeYPlacement(false),
	m_DispSpanDrawstopColToRight(false),
	m_DispAtTopOfDrawstopCol(false),
	m_DispDrawstopCol(0),
	m_DispXpos(0),
	m_DispYpos(0),
	m_DispLabelFontSize(0),
	m_DispImageNum(0),
	m_DispLabelColour(0,0,0),
	m_Name(),
	m_label(label)
{
}

void GOGUILabel::Init(IniFileConfig& cfg, unsigned x, unsigned y, wxString group)
{
	m_FreeXPlacement = cfg.ReadBoolean(group, wxT("FreeXPlacement"), false, true);
	m_FreeYPlacement = cfg.ReadBoolean(group, wxT("FreeYPlacement"), false, true);

	if (!m_FreeXPlacement)
	{
		m_DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, m_metrics->NumberOfDrawstopColsToDisplay(), false, 1);
		m_DispSpanDrawstopColToRight = cfg.ReadBoolean(group, wxT("DispSpanDrawstopColToRight"), false, false);
	}
	else
	{
		m_DispXpos = cfg.ReadInteger(group, wxT("DispXpos"), 0, m_metrics->GetScreenWidth(), false, x);
	}

	if (!m_FreeYPlacement)
	{
		m_DispAtTopOfDrawstopCol = cfg.ReadBoolean(group, wxT("DispAtTopOfDrawstopCol"), false, false);
	}
	else
	{
		m_DispYpos = cfg.ReadInteger(group, wxT("DispYpos"), 0, m_metrics->GetScreenHeight(), false, y);
	}

	/* NOTICE: this should not be allowed, but some existing definition files
	 * use improper values */
	if (!m_DispXpos)
		m_DispYpos++;
	if (!m_DispYpos)
		m_DispYpos++;

	m_DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"), false, wxT("BLACK"));
	m_DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"), false, wxT("NORMAL"));
	m_DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 1, false, 1);

	if (!m_FreeXPlacement)
	{
		int i = m_metrics->NumberOfDrawstopColsToDisplay() >> 1;
		if (m_DispDrawstopCol <= i)
			m_DispXpos = m_metrics->GetJambLeftX()  + (m_DispDrawstopCol - 1) * 78 + 1;
		else
			m_DispXpos = m_metrics->GetJambRightX() + (m_DispDrawstopCol - 1 - i) * 78 + 1;
		if (m_DispSpanDrawstopColToRight)
			m_DispXpos += 39;
	}

	if (!m_FreeYPlacement)
	{
		m_DispYpos = m_metrics->GetJambLeftRightY() + 1;
		if (!m_DispAtTopOfDrawstopCol)
			m_DispYpos += m_metrics->GetJambLeftRightHeight() - 32;
	}

	m_BoundingRect = wxRect(m_DispXpos - 1, m_DispYpos - 1, 78, 22);
}

void GOGUILabel::Load(IniFileConfig& cfg, wxString group)
{

	m_Name = cfg.ReadString(group, wxT("Name"), 64);
	m_FreeXPlacement = cfg.ReadBoolean(group, wxT("FreeXPlacement"));
	m_FreeYPlacement = cfg.ReadBoolean(group, wxT("FreeYPlacement"));

	if (!m_FreeXPlacement)
	{
		m_DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, m_metrics->NumberOfDrawstopColsToDisplay());
		m_DispSpanDrawstopColToRight = cfg.ReadBoolean(group, wxT("DispSpanDrawstopColToRight"));
	}
	else
	{
		m_DispXpos = cfg.ReadInteger(group, wxT("DispXpos"), 0, m_metrics->GetScreenWidth());
	}

	if (!m_FreeYPlacement)
	{
		m_DispAtTopOfDrawstopCol = cfg.ReadBoolean(group, wxT("DispAtTopOfDrawstopCol"));
	}
	else
	{
		m_DispYpos = cfg.ReadInteger(group, wxT("DispYpos"), 0, m_metrics->GetScreenHeight());
	}

	/* NOTICE: this should not be allowed, but some existing definition files
	 * use improper values */
	if (!m_DispXpos)
		m_DispYpos++;
	if (!m_DispYpos)
		m_DispYpos++;

	m_DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"));
	m_DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"));
	m_DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 1);

	if (!m_FreeXPlacement)
	{
		int i = m_metrics->NumberOfDrawstopColsToDisplay() >> 1;
		if (m_DispDrawstopCol <= i)
			m_DispXpos = m_metrics->GetJambLeftX()  + (m_DispDrawstopCol - 1) * 78 + 1;
		else
			m_DispXpos = m_metrics->GetJambRightX() + (m_DispDrawstopCol - 1 - i) * 78 + 1;
		if (m_DispSpanDrawstopColToRight)
			m_DispXpos += 39;
	}

	if (!m_FreeYPlacement)
	{
		m_DispYpos = m_metrics->GetJambLeftRightY() + 1;
		if (!m_DispAtTopOfDrawstopCol)
			m_DispYpos += m_metrics->GetJambLeftRightHeight() - 32;
	}

	m_BoundingRect = wxRect(m_DispXpos - 1, m_DispYpos - 1, 78, 22);
}

void GOGUILabel::Draw(wxDC* dc)
{
	if (m_label)
		m_Name = m_label->GetName();

	dc->DrawBitmap(*m_panel->GetImage(8), m_BoundingRect.GetX(), m_BoundingRect.GetY(), false);

	wxFont font = m_metrics->GetGroupLabelFont();
	font.SetPointSize(m_DispLabelFontSize);
	dc->SetTextForeground(m_DispLabelColour);
	dc->SetFont(font);
	dc->DrawLabel(m_Name, m_BoundingRect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	GOGUIControl::Draw(dc);
}
