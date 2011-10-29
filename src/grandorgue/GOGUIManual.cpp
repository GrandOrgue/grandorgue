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

#include "GOGUIManual.h"
#include "GOGUIPanel.h"
#include "GOGUIMouseState.h"
#include "IniFileConfig.h"
#include "GOGUIDisplayMetrics.h"
#include "GOrgueManual.h"
#include "MIDIEventDialog.h"

GOGUIManual::GOGUIManual(GOGUIPanel* panel, GOrgueManual* manual, unsigned manual_number):
	GOGUIControl(panel, manual),
	m_manual(manual),
	m_ManualNumber(manual_number),
	m_key_colour_inverted(false),
	m_key_colour_wooden(false)
{
}

void GOGUIManual::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
	m_key_colour_inverted               = cfg.ReadBoolean(group, wxT("DispKeyColourInverted"));
	m_key_colour_wooden                 = cfg.ReadBoolean(group, wxT("DispKeyColourWooden"), false);

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);
	m_BoundingRect = wxRect(mri.x, mri.y, mri.width, mri.height);
}

void GOGUIManual::GetKeyDimensions(unsigned key_midi_nb, int &x, int &cx, int &cy, int &z)
{
	static const int addends[12] = {0, 9, 12, 21, 24, 36, 45, 48, 57, 60, 69, 72};

	/* Key MIDI number must be valid for this manual */
	assert
		((key_midi_nb >= m_manual->GetFirstAccessibleKeyMIDINoteNumber()) &&
		 (key_midi_nb < m_manual->GetNumberOfAccessibleKeys() + m_manual->GetFirstAccessibleKeyMIDINoteNumber()));

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);

	int is_natural = (((key_midi_nb % 12) < 5 && !(key_midi_nb & 1)) || ((key_midi_nb % 12) >= 5 && (key_midi_nb & 1))) ? 0 : 1;
	int j;
	if (m_ManualNumber)
	{
		x  = mri.x + (key_midi_nb / 12) * 84;
		x += addends[key_midi_nb % 12];
		j  = m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		x -= (j / 12) * 84;
		x -= addends[j % 12];
		if (is_natural)
		{
			cx = 7;
			cy = 20;
		}
		else
		{
			cx = 13;
			cy = 32;
		}
	}
	else
	{
		cx = 8;
		x  = mri.x + (key_midi_nb / 12) * 98;
		x += (key_midi_nb % 12) * 7;
		if ((key_midi_nb % 12) >= 5)
			x += 7;
		j  = m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		x -= (j / 12) * 98;
		x -= (j % 12) * 7;
		if ((j % 12) >= 5)
			x -= 7;
		if (is_natural)
		{
			cy = 20;
		}
		else
		{
			cy = 40;
		}
	}

	if (!m_ManualNumber || is_natural)
	{
		z = -4;
	}
	else
	{
		z = 0;

		int j = key_midi_nb % 12;
		if (key_midi_nb > m_manual->GetFirstAccessibleKeyMIDINoteNumber() && j && j != 5)
			z |= 2;
		if (key_midi_nb < m_manual->GetFirstAccessibleKeyMIDINoteNumber() + m_manual->GetNumberOfAccessibleKeys() - 1 && j != 4 && j != 11)
			z |= 1;
	}


}

wxRegion GOGUIManual::GetKeyRegion(unsigned key_nb)
{

	int x, cx, cy, z;
	wxRegion reg;

	assert((key_nb >= 0) && (key_nb < m_manual->GetNumberOfAccessibleKeys())); /* the index must be valid */

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);

	key_nb += m_manual->GetFirstAccessibleKeyMIDINoteNumber();
	GetKeyDimensions(key_nb, x, cx, cy, z);

	int is_sharp  = (((key_nb % 12) < 5 && !(key_nb & 1)) || ((key_nb % 12) >= 5 && (key_nb & 1))) ? 0 : 1;

	if (!m_ManualNumber || is_sharp)
	{
		reg.Union(x, mri.keys_y, cx + 1, cy);
	}
	else
	{
		reg.Union(x + 3, mri.keys_y, 8, cy);
		reg.Union(x, mri.keys_y + 18, 14, 14);

		if (!(z & 2))
			reg.Union(x, mri.keys_y, 3, 18);
		if (!(z & 1))
			reg.Union(x + 11, mri.keys_y, 3, 18);
	}

	return reg;
}

void GOGUIManual::DrawKey(wxDC* dc, unsigned key_nb)
{

	static wxPoint g_points[4][17] = {
		{ wxPoint( 0, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
		{ wxPoint( 0, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
		{ wxPoint( 3, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
		{ wxPoint( 3, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
	};

	assert(key_nb < m_manual->GetNumberOfAccessibleKeys()); /* the index must be valid */

	int k = key_nb + m_manual->GetFirstAccessibleKeyMIDINoteNumber();
	int x, cx, cy, z;
	GetKeyDimensions(k, x, cx, cy, z);

	const wxPen* pen = m_manual->IsKeyDown(k) ? wxRED_PEN : wxGREY_PEN;
	dc->SetPen(*pen);
	wxRegion exclude;

	if (!m_manual->IsKeyDown(k))
	{

		if ((k - m_manual->GetFirstAccessibleKeyMIDINoteNumber()) > 0 && m_manual->IsKeyDown(k - 1)) {
			k -= m_manual->GetFirstAccessibleKeyMIDINoteNumber();
			exclude.Union(GetKeyRegion(k - 1));
			k += m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		}
		if ((z & 2) && (k - m_manual->GetFirstAccessibleKeyMIDINoteNumber()) > 1 && m_manual->IsKeyDown(k - 2)) {
			k -= m_manual->GetFirstAccessibleKeyMIDINoteNumber();
			exclude.Union(GetKeyRegion(k - 2));
			k += m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		}
		if ((k - m_manual->GetFirstAccessibleKeyMIDINoteNumber()) < m_manual->GetNumberOfAccessibleKeys() - 1 && m_manual->IsKeyDown(k + 1)) {
			k -= m_manual->GetFirstAccessibleKeyMIDINoteNumber();
			exclude.Union(GetKeyRegion(k + 1));
			k += m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		}
		if ((z & 1) && (k - m_manual->GetFirstAccessibleKeyMIDINoteNumber()) < m_manual->GetNumberOfAccessibleKeys() - 2 && m_manual->IsKeyDown(k + 2)) {
			k -= m_manual->GetFirstAccessibleKeyMIDINoteNumber();
			exclude.Union(GetKeyRegion(k + 2));
			k += m_manual->GetFirstAccessibleKeyMIDINoteNumber();
		}

	}

	wxRegion reg = GetKeyRegion(key_nb);

	if (!exclude.IsEmpty())
	{
		reg.Subtract(exclude);
		reg.Offset(dc->LogicalToDeviceX(0), dc->LogicalToDeviceY(0));
		dc->SetClippingRegion(reg);
	}

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);

	if (z < 0)
	{
		dc->SetBrush(*wxTRANSPARENT_BRUSH);
		dc->DrawRectangle(x, mri.keys_y, cx + 1, cy);
		dc->DrawRectangle(x + 1, mri.keys_y + 1, cx - 1, cy - 2);
	}
	else
	{
		dc->DrawPolygon(9 + (((z + 1) >> 1) << 2), g_points[z], x, mri.keys_y);
	}

	if (!exclude.IsEmpty())
		dc->DestroyClippingRegion();
}


void GOGUIManual::Draw(wxDC* dc)
{
	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_metrics->GetManualRenderInfo(m_ManualNumber);

	wxRegion region;
	for (unsigned j = 0; j < m_manual->GetNumberOfAccessibleKeys(); j++)
	{
		unsigned k = m_manual->GetFirstAccessibleKeyMIDINoteNumber() + j;
		if ( (((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			region.Union(GetKeyRegion(j));
	}

	if (!region.IsEmpty())
	{

		unsigned j = 31 + (m_key_colour_inverted << 1);
		if (j == 31 && (m_key_colour_wooden || !m_ManualNumber))
			j = 35;

		dc->SetClippingRegion(region);
		m_panel->TileWood(dc, j, m_metrics->GetCenterX(), mri.keys_y, m_metrics->GetCenterWidth(), mri.height);

	}
	region.Clear();

	for (unsigned j = 0; j < m_manual->GetNumberOfAccessibleKeys(); j++)
	{
		unsigned k = m_manual->GetFirstAccessibleKeyMIDINoteNumber() + j;
		if (!(((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			region.Union(GetKeyRegion(j));
	}

	if (!region.IsEmpty())
	{
		unsigned j = 33 - (m_key_colour_inverted << 1);
		if (j == 31 && (m_key_colour_wooden || !m_ManualNumber))
			j = (m_metrics->GetKeyVertBackgroundImageNum() % 10) == 1 && !m_ManualNumber ? 13 : 35;

		dc->SetClippingRegion(region);
		m_panel->TileWood(dc, j, m_metrics->GetCenterX(), mri.keys_y, m_metrics->GetCenterWidth(), mri.height);

	}

	for (unsigned j = 0; j < m_manual->GetNumberOfAccessibleKeys(); j++)
		DrawKey(dc, j);
	GOGUIControl::Draw(dc);
}

void GOGUIManual::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	if (!m_BoundingRect.Contains(x, y))
		return;

	if (right)
	{
		GOrgueMidiReceiver& m_midi = m_manual->GetMidiReceiver();
		MIDIEventDialog dlg (m_panel->GetParentWindow(), _("Midi-Settings for Manual - ") + m_manual->GetName(), m_midi);

		if (dlg.ShowModal() == wxID_OK)
		{
			m_midi = dlg.GetResult();
			m_panel->Modified();
			m_manual->AllNotesOff();
		}
	}
	else
	{
		wxRegion reg;
		for(unsigned i = 0; i < m_manual->GetNumberOfAccessibleKeys(); i++)
		{
			reg = GetKeyRegion(i);
			if (reg.Contains(x, y))
			{
				if (state.GetControl() == this && state.GetIndex() == i)
					return;
				state.SetControl(this);
				state.SetIndex(i);

				m_manual->Set(i + m_manual->GetFirstAccessibleKeyMIDINoteNumber(), !m_manual->IsKeyDown(i + m_manual->GetFirstAccessibleKeyMIDINoteNumber()));
				return;
			}
		}
	}
}

