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

#include "GOGUIDrawStop.h"
#include "GOGUIPanel.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrgueDrawStop.h"
#include "MIDIEventDialog.h"

GOGUIDrawstop::GOGUIDrawstop(GOGUIPanel* panel, GOrgueDrawstop* control, unsigned x_pos, unsigned y_pos):
	GOGUIControl(panel, control),
	m_drawstop(control),
	m_ShortcutKey(0),
	m_DispKeyLabelOnLeft(false),
	m_DispLabelFontSize(0),
	m_DispLabelColour(0,0,0),
	m_DispDrawstopRow(y_pos),
	m_DispDrawstopCol(x_pos),
	m_DispImageNum(0)
{
}

void GOGUIDrawstop::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
	m_ShortcutKey = cfg.ReadInteger(group, wxT("ShortcutKey"), 0, 255, false, 0);
	m_DispKeyLabelOnLeft = cfg.ReadBoolean(group, wxT("DispKeyLabelOnLeft"), true, false);
	m_DispLabelColour = cfg.ReadColor(group, wxT("DispLabelColour"), true, wxT("Dark Red"));
	m_DispLabelFontSize = cfg.ReadFontSize(group, wxT("DispLabelFontSize"), true, wxT("normal"));
	m_DispDrawstopRow = cfg.ReadInteger(group, wxT("DispDrawstopRow"), 1, 99 + m_metrics->NumberOfExtraDrawstopRowsToDisplay(), true, m_DispDrawstopRow);
	m_DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, m_DispDrawstopRow > 99 ? m_metrics->NumberOfExtraDrawstopColsToDisplay() : m_metrics->NumberOfDrawstopColsToDisplay(), true, m_DispDrawstopCol);
	m_DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2, true, 1);

	int x, y;
	m_metrics->GetDrawstopBlitPosition(m_DispDrawstopRow, m_DispDrawstopCol, &x, &y);
	m_BoundingRect = wxRect(x, y, 65, 65);
}

void GOGUIDrawstop::Draw(wxDC* dc)
{
	wxRect rect(m_BoundingRect.GetX(), m_BoundingRect.GetY() + 1, m_BoundingRect.GetWidth(), m_BoundingRect.GetHeight() - 1);
	wxBitmap* bmp = m_panel->GetImage(((m_DispImageNum - 1) << 1) + (m_drawstop->DisplayInverted() ^ m_drawstop->IsEngaged() ? 1 : 0));
	dc->DrawBitmap(*bmp, m_BoundingRect.GetX(), m_BoundingRect.GetY(), true);
	dc->SetTextForeground(m_DispLabelColour);
	wxFont font = m_metrics->GetControlLabelFont();
	font.SetPointSize(m_DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(m_panel->WrapText(dc, m_drawstop->GetName(), 51), rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	GOGUIControl::Draw(dc);
}

void GOGUIDrawstop::HandleMousePress(int x, int y, bool right)
{
	if (!m_BoundingRect.Contains(x, y))
		return;
	if ((m_BoundingRect.GetX() + 32 - x) * (m_BoundingRect.GetX() + 32 - x) + (m_BoundingRect.GetY() + 32 - y) * (m_BoundingRect.GetY() + 32 - y) > 1024)
		return;
	if (right)
	{
		GOrgueMidiReceiver& m_midi = m_drawstop->GetMidiReceiver();
		MIDIEventDialog dlg (m_panel->GetParentWindow(), _("Midi-Settings for Drawstop - ") + m_drawstop->GetName(), m_midi);
		
		if (dlg.ShowModal() == wxID_OK)
		{
			m_midi = dlg.GetResult();
			m_panel->Modified();
		}
	}
	else
		m_drawstop->Push();
}

void GOGUIDrawstop::HandleKey(int key)
{
	if (key == m_ShortcutKey)
		m_drawstop->Push();
}
