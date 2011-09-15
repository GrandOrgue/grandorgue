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

#include "GOGUISetterButton.h"
#include "GOGUIPanel.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrgueSetterButton.h"
#include "MIDIEventDialog.h"
#include "IniFileConfig.h"

GOGUISetterButton::GOGUISetterButton(GOGUIPanel* panel, GOrgueSetterButton* control):
	GOGUIControl(panel, control),
	m_button(control),
	m_DispKeyLabelOnLeft(false),
	m_DispLabelFontSize(0),
	m_DispLabelColour(0,0,0),
	m_DispDrawstopRow(0),
	m_DispDrawstopCol(0),
	m_DispImageNum(0),
	m_Piston(false)
{
}

void GOGUISetterButton::Init(IniFileConfig& cfg, unsigned x_pos, unsigned y_pos, wxString group, bool IsPiston)
{
	m_Piston = cfg.ReadBoolean(group, wxT("DisplayAsButton"), false, IsPiston);
	m_DispKeyLabelOnLeft = cfg.ReadBoolean(group, wxT("DispKeyLabelOnLeft"), false, true);
	m_DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"), false, wxT("Dark Red"));
	m_DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"), false, wxT("normal"));
	m_DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2, false, 1);
	if (!m_Piston)
	{
		m_DispDrawstopRow = cfg.ReadInteger(group, wxT("DispDrawstopRow"), 1, 99 + m_metrics->NumberOfExtraDrawstopRowsToDisplay(), false, y_pos);
		m_DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, m_DispDrawstopRow > 99 ? m_metrics->NumberOfExtraDrawstopColsToDisplay() : m_metrics->NumberOfDrawstopColsToDisplay(), false, x_pos);
	}
	else
	{
		m_DispDrawstopRow = cfg.ReadInteger(group, wxT("DispButtonRow"), 1, 99 + m_metrics->NumberOfExtraButtonRows(), false, y_pos);
		m_DispDrawstopCol = cfg.ReadInteger(group, wxT("DispButtonCol"), 1, m_metrics->NumberOfButtonCols(), false, x_pos);
		m_DispImageNum--;
	}

	int x, y;
	if (m_Piston)
	{
		m_metrics->GetPushbuttonBlitPosition(m_DispDrawstopRow, m_DispDrawstopCol, &x, &y);
		m_BoundingRect = wxRect(x, y, 30, 30);
	}
	else
	{
		m_metrics->GetDrawstopBlitPosition(m_DispDrawstopRow, m_DispDrawstopCol, &x, &y);
		m_BoundingRect = wxRect(x, y, 65, 65);
	}
}

void GOGUISetterButton::Draw(wxDC* dc)
{
	wxRect rect(m_BoundingRect.GetX(), m_BoundingRect.GetY() + 1, m_BoundingRect.GetWidth(), m_BoundingRect.GetHeight() - 1);
	wxBitmap* bmp;
	if (m_Piston)
		bmp = m_panel->GetImage(m_DispImageNum + 4 + (m_button->IsEngaged() ? 2 : 0));
	else
		bmp = m_panel->GetImage(((m_DispImageNum - 1) << 1) + (m_button->IsEngaged() ? 1 : 0));
	dc->DrawBitmap(*bmp, m_BoundingRect.GetX(), m_BoundingRect.GetY(), true);
	dc->SetTextForeground(m_DispLabelColour);
	wxFont font = m_metrics->GetControlLabelFont();
	font.SetPointSize(m_DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(m_panel->WrapText(dc, m_button->GetName(), m_Piston ? 28 : 51), rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
}

void GOGUISetterButton::HandleMousePress(int x, int y, bool right)
{
	if (!m_BoundingRect.Contains(x, y))
		return;
	if (!m_Piston && (m_BoundingRect.GetX() + 32 - x) * (m_BoundingRect.GetX() + 32 - x) + (m_BoundingRect.GetY() + 32 - y) * (m_BoundingRect.GetY() + 32 - y) > 1024)
		return;
	if (m_Piston && (m_BoundingRect.GetX() + 15 - x) * (m_BoundingRect.GetX() + 15 - x) + (m_BoundingRect.GetY() + 15 - y) * (m_BoundingRect.GetY() + 15 - y) > 255)
		return;
	if (right)
	{
		GOrgueMidiReceiver& m_midi = m_button->GetMidiReceiver();
		MIDIEventDialog dlg (m_panel->GetParentWindow(), _("Midi-Settings for Button - ") + m_button->GetName(), m_midi);
		
		if (dlg.ShowModal() == wxID_OK)
		{
			m_midi = dlg.GetResult();
			m_panel->Modified();
		}
	}
	else
		m_button->Push();
}
