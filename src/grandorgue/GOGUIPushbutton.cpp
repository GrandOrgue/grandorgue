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

#include "GOGUIPushbutton.h"
#include "GOGUIPanel.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrguePushbutton.h"
#include "MIDIEventDialog.h"
#include "IniFileConfig.h"

GOGUIPushbutton::GOGUIPushbutton(GOGUIPanel* panel, GOrguePushbutton* control, unsigned x_pos, unsigned y_pos):
	GOGUIControl(panel, control),
	m_pushbutton(control),
	m_ShortcutKey(0),
	m_DispKeyLabelOnLeft(false),
	m_DispLabelFontSize(0),
	m_DispLabelColour(0,0,0),
	m_DispButtonRow(y_pos),
	m_DispButtonCol(x_pos),
	m_DispImageNum(0)
{
}

void GOGUIPushbutton::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
	m_ShortcutKey = cfg.ReadInteger(group, wxT("ShortcutKey"), 0, 255, false, 0);
	m_DispKeyLabelOnLeft = cfg.ReadBoolean(group, wxT("DispKeyLabelOnLeft"), true, true);
	m_DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"), true, wxT("Dark Red"));
	m_DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"), true, wxT("normal"));
	m_DispButtonRow = cfg.ReadInteger(group, wxT("DispButtonRow"), 0, 99 + m_metrics->NumberOfExtraButtonRows(), true, m_DispButtonRow);
	m_DispButtonCol = cfg.ReadInteger(group, wxT("DispButtonCol"), 1, m_metrics->NumberOfButtonCols(), true, m_DispButtonCol);
	m_DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2, true, 1);
	m_DispImageNum--;

	int x, y;
	m_metrics->GetPushbuttonBlitPosition(m_DispButtonRow, m_DispButtonCol, &x, &y);
	if (!m_DispKeyLabelOnLeft)
		x -= 13;

	m_BoundingRect = wxRect(x, y, 30, 30);
}

void GOGUIPushbutton::Draw(wxDC* dc)
{
	wxMemoryDC mdc;
	wxRect rect(m_BoundingRect.GetX() + 1, m_BoundingRect.GetY() + 1, m_BoundingRect.GetWidth() - 1, m_BoundingRect.GetHeight() - 1);
	wxBitmap* bmp = m_panel->GetImage(m_DispImageNum + 4 + (m_pushbutton->IsPushed() ? 2 : 0));
	dc->DrawBitmap(*bmp, m_BoundingRect.GetX(), m_BoundingRect.GetY(), true);
	dc->SetTextForeground(m_DispLabelColour);
	wxFont font = m_metrics->GetControlLabelFont();
	font.SetPointSize(m_DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(m_panel->WrapText(dc, m_pushbutton->GetName(), 28), rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	GOGUIControl::Draw(dc);
}

void GOGUIPushbutton::HandleMousePress(int x, int y, bool right)
{
	if (!m_BoundingRect.Contains(x, y))
		return;
	if ((m_BoundingRect.GetX() + 15 - x) * (m_BoundingRect.GetX() + 15 - x) + (m_BoundingRect.GetY() + 15 - y) * (m_BoundingRect.GetY() + 15 - y) > 255)
		return;
	if (right)
	{
		GOrgueMidiReceiver& m_midi = m_pushbutton->GetMidiReceiver();
		MIDIEventDialog dlg (m_panel->GetParentWindow(), _("Midi-Settings for Pushbutton - ") + m_pushbutton->GetName(), m_midi);
		
		if (dlg.ShowModal() == wxID_OK)
		{
			m_midi = dlg.GetResult();
			m_panel->Modified();
		}
	}
	else
		m_pushbutton->Push();
}

void GOGUIPushbutton::HandleKey(int key)
{
	if (key == m_ShortcutKey)
		m_pushbutton->Push();
}
