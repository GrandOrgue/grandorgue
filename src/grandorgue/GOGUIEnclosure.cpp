/*
 * GrandOrgue - free pipe organ simulator
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

#include "GOGUIEnclosure.h"
#include "GOrgueEnclosure.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUIMouseState.h"
#include "GOGUIPanel.h"
#include "MIDIEventDialog.h"

GOGUIEnclosure::GOGUIEnclosure(GOGUIPanel* panel, GOrgueEnclosure* control, unsigned enclosure_nb):
	GOGUIControl(panel, control),
	m_enclosure(control),
	m_enclosure_nb(enclosure_nb)
{
}

bool GOGUIEnclosure::IsEnclosure(const unsigned nb) const
{
	return (m_enclosure_nb == nb);
}

void GOGUIEnclosure::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);

	int x = m_metrics->GetEnclosureX(this);
	int y = m_metrics->GetEnclosureY();
	m_BoundingRect = wxRect(x, y, 46, 61);
}

void GOGUIEnclosure::Draw(wxDC* dc)
{
	dc->SetPen(*wxTRANSPARENT_PEN);
	dc->SetBrush(*wxBLACK_BRUSH);

	wxRect rect(m_BoundingRect);
	dc->DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	wxFont font = *wxNORMAL_FONT;
	font.SetPointSize(7);
	dc->SetFont(font);
	dc->SetTextForeground(*wxWHITE);

	dc->DrawLabel(m_enclosure->GetName(), rect, wxALIGN_CENTER_HORIZONTAL);

	int enclosure_x = m_BoundingRect.GetX();
	int enclosure_y = m_BoundingRect.GetY();

	dc->SetBrush(*wxBLACK_BRUSH);
	dc->DrawRectangle(enclosure_x, enclosure_y + 13, 46, 44);
	int dx = 1 + ( 3 * m_enclosure->GetValue()) / 127;
	int dy = 1 + (13 * m_enclosure->GetValue()) / 127;
	wxPoint points[4];
	points[0].x = enclosure_x +  7 + dx;
	points[1].x = enclosure_x + 38 - dx;
	points[2].x = enclosure_x + 38 + dx;
	points[3].x = enclosure_x +  7 - dx;
	points[0].y = points[1].y = enclosure_y + 13 + dy;
	points[2].y = points[3].y = enclosure_y + 56 - dy;
	dc->SetBrush(m_metrics->GetPedalBrush());
	dc->DrawPolygon(4, points);
	GOGUIControl::Draw(dc);
}


void GOGUIEnclosure::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	if (!m_BoundingRect.Contains(x, y))
		return;
	if (right)
	{
		GOrgueMidiReceiver& m_midi = m_enclosure->GetMidiReceiver();
		MIDIEventDialog dlg (m_panel->GetParentWindow(), _("Midi-Settings for Enclosure - ") + m_enclosure->GetName(), m_midi);
		
		if (dlg.ShowModal() == wxID_OK)
		{
			m_midi = dlg.GetResult();
			m_panel->Modified();
		}
	}
	else
	{
		y -= m_BoundingRect.GetY() + 13;
		if (y < 0 || y > 45)
			return;
		if (y > 16 && y < 29 )
			y = 16;
		else if (y >= 30)
			y = 45 - y;

		unsigned value = y;

		if (state.GetControl() == this && state.GetIndex() == value)
			return;
		state.SetControl(this);
		state.SetIndex(value);

		m_enclosure->Set(value * 8);
	}
}

void GOGUIEnclosure::HandleMouseScroll(int x, int y, int amount)
{
	if (!m_BoundingRect.Contains(x, y) || !amount)
		return;
	m_enclosure->Scroll(amount > 0);
}
