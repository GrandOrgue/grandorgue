/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
	m_enclosure_nb(enclosure_nb),
	m_MouseAxisStart(0),
	m_MouseAxisEnd(0),
	m_TileOffsetX(0),
	m_TileOffsetY(0),
	m_Bitmaps(0)
{
}

bool GOGUIEnclosure::IsEnclosure(const unsigned nb) const
{
	return (m_enclosure_nb == nb);
}

void GOGUIEnclosure::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);

	for(int i = 0; i < 15; i++)
		m_Bitmaps.push_back(m_panel->LoadBitmap(wxString::Format(wxT("GO:enclosure%d"), i), wxEmptyString));

	int x = m_metrics->GetEnclosureX(this);
	int y = m_metrics->GetEnclosureY();
	m_BoundingRect = wxRect(x, y, 46, 61);
	m_TextRect = wxRect(x, y, 46, 61);
	m_MouseRect = wxRect(x, y + 13, 46, 45);
	m_MouseAxisStart = 16;
	m_MouseAxisEnd = 30;
}

void GOGUIEnclosure::Draw(wxDC* dc)
{
	wxBitmap* bmp = m_Bitmaps[((m_Bitmaps.size() - 1) * m_enclosure->GetValue()) / 127];
	m_panel->TileBitmap(dc, bmp, m_BoundingRect, m_TileOffsetX, m_TileOffsetY);

	wxFont font = *wxNORMAL_FONT;
	font.SetPointSize(7);
	dc->SetFont(font);
	dc->SetTextForeground(*wxWHITE);

	dc->DrawLabel(m_enclosure->GetName(), m_TextRect, wxALIGN_CENTER_HORIZONTAL);

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
		unsigned value;
		if (!m_MouseRect.Contains(x, y))
			return;
		y -= m_MouseRect.GetY();
		if (y <= m_MouseAxisStart)
			value = (127 * y / m_MouseAxisStart);
		else if (y >= m_MouseAxisEnd)
			value = (m_MouseRect.GetHeight() - y) * 127 / (m_MouseRect.GetHeight() - m_MouseAxisEnd);
		else
			value = 127;

		if (state.GetControl() == this && state.GetIndex() == value)
			return;
		state.SetControl(this);
		state.SetIndex(value);

		m_enclosure->Set(value);
	}
}

void GOGUIEnclosure::HandleMouseScroll(int x, int y, int amount)
{
	if (!m_BoundingRect.Contains(x, y) || !amount)
		return;
	m_enclosure->Scroll(amount > 0);
}
