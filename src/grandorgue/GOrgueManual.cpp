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
#include "GOrgueManual.h"

#include <algorithm>
#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrguePipe.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "IniFileConfig.h"
#include "MIDIEventDialog.h"
#include "GOrgueMidi.h"
#include "OrganPanel.h"
#include "GOGUIDisplayMetrics.h"

GOrgueManual::GOrgueManual(GrandOrgueFile* organfile) :
	m_group(wxT("---")),
	m_midi(organfile, MIDI_RECV_MANUAL),
	m_organfile(organfile),
	m_KeyPressed(0),
	m_KeyState(0),
	m_manual_number(0),
	m_first_accessible_logical_key_nb(0),
	m_nb_logical_keys(0),
	m_first_accessible_key_midi_note_nb(0),
	m_nb_accessible_keys(0),
	m_UnisonOff(0),
	m_midi_input_number(0),
	m_tremulant_ids(0),
	m_name(),
	m_stops(0),
	m_couplers(0),
	m_divisionals(0),
	m_displayed(false),
	m_key_colour_inverted(false),
	m_key_colour_wooden(false)
{

}

void GOrgueManual::Load(IniFileConfig& cfg, wxString group, GOGUIDisplayMetrics* displayMetrics, int manualNumber)
{
	m_group = group;
	m_name                              = cfg.ReadString (group, wxT("Name"), 32);
	m_nb_logical_keys                   = cfg.ReadInteger(group, wxT("NumberOfLogicalKeys"), 1, 192);
	m_first_accessible_logical_key_nb   = cfg.ReadInteger(group, wxT("FirstAccessibleKeyLogicalKeyNumber"), 1, m_nb_logical_keys);
	m_first_accessible_key_midi_note_nb = cfg.ReadInteger(group, wxT("FirstAccessibleKeyMIDINoteNumber"), 0, 127);
	m_nb_accessible_keys                = cfg.ReadInteger(group, wxT("NumberOfAccessibleKeys"), 0, 85);
	m_midi_input_number                 = cfg.ReadInteger(group, wxT("MIDIInputNumber"), 1, 6);
	m_displayed                         = cfg.ReadBoolean(group, wxT("Displayed"));
	m_key_colour_inverted               = cfg.ReadBoolean(group, wxT("DispKeyColourInverted"));
	m_key_colour_wooden                 = cfg.ReadBoolean(group, wxT("DispKeyColourWooden"), false);
	unsigned m_nb_stops                 = cfg.ReadInteger(group, wxT("NumberOfStops"), 0, 64);
	unsigned m_nb_couplers              = cfg.ReadInteger(group, wxT("NumberOfCouplers"), 0, 16, false);
	unsigned m_nb_divisionals           = cfg.ReadInteger(group, wxT("NumberOfDivisionals"), 0, 32, false);
	unsigned m_nb_tremulants            = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, 10, false);
	m_manual_number = manualNumber;
	m_display_metrics = displayMetrics;

	m_midi.SetManual(manualNumber);

	wxString buffer;

	m_stops.resize(0);
	for (unsigned i = 0; i < m_nb_stops; i++)
	{
		m_stops.push_back(new GOrgueStop(m_organfile, m_manual_number));
		buffer.Printf(wxT("Stop%03d"), i + 1);
		buffer.Printf(wxT("Stop%03d"), cfg.ReadInteger(group, buffer, 1, 448));
		m_stops[i]->Load(cfg, buffer, displayMetrics);
	}

	m_couplers.resize(0);
	for (unsigned i = 0; i < m_nb_couplers; i++)
	{
		m_couplers.push_back(new GOrgueCoupler(m_organfile, m_manual_number));
		buffer.Printf(wxT("Coupler%03d"), i + 1);
		buffer.Printf(wxT("Coupler%03d"), cfg.ReadInteger(group, buffer, 1, 64));
		m_couplers[i]->Load(cfg, buffer, displayMetrics);
	}

	m_tremulant_ids.resize(0);
	for (unsigned i = 0; i < m_nb_tremulants; i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant_ids.push_back(cfg.ReadInteger(group, buffer, 1, m_organfile->GetTremulantCount()));
	}

	m_divisionals.resize(0);
	for (unsigned i = 0; i < m_nb_divisionals; i++)
	{
		m_divisionals.push_back(new GOrgueDivisional(m_organfile));
		buffer.Printf(wxT("Divisional%03d"), i + 1);
		buffer.Printf(wxT("Divisional%03d"), cfg.ReadInteger(group, buffer, 1, 224));
		m_divisionals[i]->Load(cfg, buffer, m_manual_number, i, displayMetrics);
	}
	m_midi.Load(cfg, group);

	m_KeyState.resize(m_nb_logical_keys);
	std::fill(m_KeyState.begin(), m_KeyState.end(), 0);
	m_KeyPressed.resize(m_nb_accessible_keys);
	std::fill(m_KeyPressed.begin(), m_KeyPressed.end(), false);
}

void GOrgueManual::SetKey(unsigned note, int on, GOrgueCoupler* prev)
{
	if (note < 0 || note >= m_KeyState.size() || !on)
		return;

	m_KeyState[note] += on;

	bool unisonoff = false;
	for (unsigned i = 0; i < m_couplers.size(); i++)
	{
		if (m_couplers[i]->IsUnisonOff() && !prev && m_couplers[i]->IsEngaged())
			unisonoff = true;
		m_couplers[i]->SetKey(note, on, prev);
	}

	if (!unisonoff)
	{
		for (unsigned i = 0; i < m_stops.size(); i++)
			m_stops[i]->SetKey(note + 1, on);
	}

	if (m_first_accessible_logical_key_nb <= note + 1 && note <= m_first_accessible_logical_key_nb + m_nb_accessible_keys)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_manual_number);
		event.SetExtraLong(note - m_first_accessible_logical_key_nb + 1);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}

#define TRIGGER_LEVEL (2<<9)

void GOrgueManual::Set(unsigned note, bool on)
{
	if (note < m_first_accessible_key_midi_note_nb || note >= m_first_accessible_key_midi_note_nb + m_KeyPressed.size())
		return;
	if (m_KeyPressed[note - m_first_accessible_key_midi_note_nb] == on)
		return;
	m_KeyPressed[note - m_first_accessible_key_midi_note_nb] = on;
	SetKey(note - m_first_accessible_key_midi_note_nb + m_first_accessible_logical_key_nb - 1, on ? TRIGGER_LEVEL : -(TRIGGER_LEVEL), NULL);
}

void GOrgueManual::SetUnisonOff(bool on)
{
	if (on)
	{
		if (m_UnisonOff++)
			return;
	}
	else
	{
		if (--m_UnisonOff)
			return;
	}
	for(unsigned i = 0; i < m_KeyPressed.size(); i++)
	{
		int note = i + m_first_accessible_logical_key_nb - 1;
		if (m_KeyPressed[note])
			for (unsigned j = 0; j < m_stops.size(); j++)
				m_stops[j]->SetKey(note + 1, on ? -TRIGGER_LEVEL : TRIGGER_LEVEL);
	}
}

void GOrgueManual::MIDI(void)
{
	MIDIEventDialog dlg (::wxGetApp().frame, _("Midi-Settings for Manual - ")+m_name ,m_midi);

	if (dlg.ShowModal() == wxID_OK)
	{
		m_midi = dlg.GetResult();
		m_organfile->Modified();
		AllNotesOff();
	}

}

GOrgueManual::~GOrgueManual(void)
{
}

GOrgueMidiReceiver& GOrgueManual::GetMidiReceiver()
{
	return m_midi;
}

const wxString& GOrgueManual::GetName()
{
	return m_name;
}

int GOrgueManual::GetMIDIInputNumber()
{
	return m_midi_input_number;
}

unsigned GOrgueManual::GetLogicalKeyCount()
{
	return m_nb_logical_keys;
}

unsigned GOrgueManual::GetNumberOfAccessibleKeys()
{
	return m_nb_accessible_keys;
}

/* TODO: I suspect this could be made private or into something better... */
unsigned GOrgueManual::GetFirstAccessibleKeyMIDINoteNumber()
{
	return m_first_accessible_key_midi_note_nb;
}

int GOrgueManual::GetFirstLogicalKeyMIDINoteNumber()
{
	return m_first_accessible_key_midi_note_nb - m_first_accessible_logical_key_nb + 1;
}

unsigned GOrgueManual::GetStopCount()
{
	return m_stops.size();
}

GOrgueStop* GOrgueManual::GetStop(unsigned index)
{
	assert(index < m_stops.size());
	return m_stops[index];
}

unsigned GOrgueManual::GetCouplerCount()
{
	return m_couplers.size();
}

GOrgueCoupler* GOrgueManual::GetCoupler(unsigned index)
{
	assert(index < m_couplers.size());
	return m_couplers[index];
}

unsigned GOrgueManual::GetDivisionalCount()
{
	return m_divisionals.size();
}

GOrgueDivisional* GOrgueManual::GetDivisional(unsigned index)
{
	assert(index < m_divisionals.size());
	return m_divisionals[index];
}

unsigned GOrgueManual::GetTremulantCount()
{
	return m_tremulant_ids.size();
}

GOrgueTremulant* GOrgueManual::GetTremulant(unsigned index)
{
	assert(index < m_tremulant_ids.size());
	return m_organfile->GetTremulant(m_tremulant_ids[index] - 1);
}

void GOrgueManual::AllNotesOff()
{

	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
		Set(m_first_accessible_key_midi_note_nb + j, false);

	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_manual_number);
		event.SetExtraLong(j);
		::wxGetApp().frame->AddPendingEvent(event);
	}

}

bool GOrgueManual::IsKeyDown(unsigned midiNoteNumber)
{
	if (midiNoteNumber < m_first_accessible_key_midi_note_nb)
		return false;
	if (midiNoteNumber >= m_first_accessible_key_midi_note_nb + m_nb_accessible_keys)
		return false;
	return m_KeyState[midiNoteNumber - m_first_accessible_key_midi_note_nb + m_first_accessible_logical_key_nb - 1] > 0;
}

void GOrgueManual::GetKeyDimensions
	(const int key_midi_nb
	,int &x
	,int &cx
	,int &cy
	,int &z
	)
{

	static const int addends[12] = {0, 9, 12, 21, 24, 36, 45, 48, 57, 60, 69, 72};

	/* Key MIDI number must be valid for this manual */
	assert
		(
			(key_midi_nb >= m_first_accessible_key_midi_note_nb)
			&&
			(key_midi_nb < (int)m_nb_accessible_keys + m_first_accessible_key_midi_note_nb)
		);

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

	int is_natural = (((key_midi_nb % 12) < 5 && !(key_midi_nb & 1)) || ((key_midi_nb % 12) >= 5 && (key_midi_nb & 1))) ? 0 : 1;
	int j;
	if (m_manual_number)
	{
		x  = mri.x + (key_midi_nb / 12) * 84;
		x += addends[key_midi_nb % 12];
		j  = m_first_accessible_key_midi_note_nb;
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
		j  = m_first_accessible_key_midi_note_nb;
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

	if (!m_manual_number || is_natural)
	{
		z = -4;
	}
	else
	{
		z = 0;

		int j = key_midi_nb % 12;
		if (key_midi_nb > (int)m_first_accessible_key_midi_note_nb && j && j != 5)
			z |= 2;
		if (key_midi_nb < (int)m_first_accessible_key_midi_note_nb + (int)m_nb_accessible_keys - 1 && j != 4 && j != 11)
			z |= 1;
	}


}

wxRegion GOrgueManual::GetKeyRegion
	(unsigned key_nb
	)
{

	int x, cx, cy, z;
	wxRegion reg;

	assert((key_nb >= 0) && (key_nb < (int)m_nb_accessible_keys)); /* the index must be valid */

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

	key_nb += m_first_accessible_key_midi_note_nb;
	GetKeyDimensions(key_nb, x, cx, cy, z);

	int is_sharp  = (((key_nb % 12) < 5 && !(key_nb & 1)) || ((key_nb % 12) >= 5 && (key_nb & 1))) ? 0 : 1;

	if (!m_manual_number || is_sharp)
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

void GOrgueManual::Click(unsigned xx, unsigned yy)
{
	wxRegion reg;
	for(unsigned i = 0; i < m_nb_accessible_keys; i++)
	{
		reg = GetKeyRegion(i);
		if (reg.Contains(xx, yy))
		{
			Set(i + m_first_accessible_key_midi_note_nb, !IsKeyDown(i + m_first_accessible_key_midi_note_nb));
			return;
		}
	}
}

void GOrgueManual::DrawKey
	(wxDC& dc
	,unsigned key_nb)
{

	static wxPoint g_points[4][17] = {
		{ wxPoint( 0, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
		{ wxPoint( 0, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1, 1), },
		{ wxPoint( 3, 0), wxPoint(13, 0), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint(12, 1), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
		{ wxPoint( 3, 0), wxPoint(10, 0), wxPoint(10,18), wxPoint(13,18), wxPoint(13,31), wxPoint( 0,31), wxPoint( 0,18), wxPoint( 3,18), wxPoint( 3, 1), wxPoint( 9, 1), wxPoint( 9,19), wxPoint(12,19), wxPoint(12,30), wxPoint( 1,30), wxPoint( 1,19), wxPoint( 4,19), wxPoint( 4, 1), },
	};

	assert(key_nb < m_nb_accessible_keys); /* the index must be valid */
	assert(m_displayed);

	int k = key_nb + m_first_accessible_key_midi_note_nb;
	int x, cx, cy, z;
	GetKeyDimensions(k, x, cx, cy, z);

	const wxPen* pen = IsKeyDown(k) ? wxRED_PEN : wxGREY_PEN;
	dc.SetPen(*pen);
	wxRegion exclude;

	if ((k - m_first_accessible_key_midi_note_nb) > 0 && IsKeyDown(k - 1)) {
		k -= m_first_accessible_key_midi_note_nb;
		exclude.Union(GetKeyRegion(k - 1));
		k += m_first_accessible_key_midi_note_nb;
	}
	if ((z & 2) && (k - m_first_accessible_key_midi_note_nb) > 1 && IsKeyDown(k - 2)) {
		k -= m_first_accessible_key_midi_note_nb;
		exclude.Union(GetKeyRegion(k - 2));
		k += m_first_accessible_key_midi_note_nb;
	}
	if ((k - m_first_accessible_key_midi_note_nb) < m_nb_accessible_keys - 1 && IsKeyDown(k + 1)) {
		k -= m_first_accessible_key_midi_note_nb;
		exclude.Union(GetKeyRegion(k + 1));
		k += m_first_accessible_key_midi_note_nb;
	}
	if ((z & 1) && (k - m_first_accessible_key_midi_note_nb) < m_nb_accessible_keys - 2 && IsKeyDown(k + 2)) {
		k -= m_first_accessible_key_midi_note_nb;
		exclude.Union(GetKeyRegion(k + 2));
		k += m_first_accessible_key_midi_note_nb;
	}

	wxRegion reg = GetKeyRegion(key_nb);

	if (!exclude.IsEmpty())
	{
		reg.Subtract(exclude);
		reg.Offset(dc.LogicalToDeviceX(0), dc.LogicalToDeviceY(0));
		dc.SetClippingRegion(reg);
	}

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

	if (z < 0)
	{
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(x, mri.keys_y, cx + 1, cy);
		dc.DrawRectangle(x + 1, mri.keys_y + 1, cx - 1, cy - 2);
	}
	else
	{
		dc.DrawPolygon(9 + (((z + 1) >> 1) << 2), g_points[z], x, mri.keys_y);
	}

	if (!exclude.IsEmpty())
		dc.DestroyClippingRegion();

}

void GOrgueManual::Draw(wxDC& dc)
{

	assert(m_displayed);

	const GOGUIDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

	OrganPanel::TileWood
		(dc
		,m_display_metrics->GetKeyVertBackgroundImageNum()
		,m_display_metrics->GetCenterX()
		,mri.y
		,m_display_metrics->GetCenterWidth()
		,mri.height
		);

	OrganPanel::TileWood
		(dc
		,m_display_metrics->GetKeyHorizBackgroundImageNum()
		,m_display_metrics->GetCenterX()
		,mri.piston_y
		,m_display_metrics->GetCenterWidth()
		,(!m_manual_number && m_display_metrics->HasExtraPedalButtonRow()) ? 80 : 40
		);

	if (m_manual_number < m_organfile->GetFirstManualIndex())
		return;

	wxFont font = m_display_metrics->GetControlLabelFont();
	for (unsigned j = 0; j < m_stops.size(); j++)
	{
		if (m_stops[j]->Displayed)
		{
			font.SetPointSize(m_stops[j]->DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_stops[j]->Name, 51);
		}
	}

	for (unsigned j = 0; j < m_couplers.size(); j++)
	{
		if (m_couplers[j]->Displayed)
		{
			font.SetPointSize(m_couplers[j]->DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_couplers[j]->Name, 51);
		}
	}

	for (unsigned j = 0; j < m_divisionals.size(); j++)
	{
		if (m_divisionals[j]->Displayed)
		{
			font.SetPointSize(m_divisionals[j]->DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_divisionals[j]->Name, 28);
		}
	}

	wxRegion region;
	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
	{
		unsigned k = m_first_accessible_key_midi_note_nb + j;
		if ( (((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			region.Union(GetKeyRegion(j));
	}

	if (!region.IsEmpty())
	{

		unsigned j = 31 + (m_key_colour_inverted << 1);
		if (j == 31 && (m_key_colour_wooden || !m_manual_number))
			j = 35;

		dc.SetClippingRegion(region);
		OrganPanel::TileWood
			(dc
			,j
			,m_display_metrics->GetCenterX()
			,mri.keys_y
			,m_display_metrics->GetCenterWidth()
			,mri.height
			);

	}
	region.Clear();

	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
	{
		unsigned k = m_first_accessible_key_midi_note_nb + j;
		if (!(((k % 12) < 5 && !(k & 1)) || ((k % 12) >= 5 && (k & 1))))
			region.Union(GetKeyRegion(j));
	}

	if (!region.IsEmpty())
	{

		unsigned j = 33 - (m_key_colour_inverted << 1);
		if (j == 31 && (m_key_colour_wooden || !m_manual_number))
			j = (m_display_metrics->GetKeyVertBackgroundImageNum() % 10) == 1 && !m_manual_number ? 13 : 35;

		dc.SetClippingRegion(region);
		OrganPanel::TileWood
			(dc
			,j
			,m_display_metrics->GetCenterX()
			,mri.keys_y
			,m_display_metrics->GetCenterWidth()
			,mri.height
			);

	}

	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
		DrawKey(dc, j);

}

bool GOrgueManual::IsDisplayed()
{
	return m_displayed;
}

void GOrgueManual::Save(IniFileConfig& cfg, bool prefix)
{
	wxString buffer;

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->Save(cfg, prefix);

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->Save(cfg, prefix);

	for (unsigned i = 0; i < m_divisionals.size(); i++)
		m_divisionals[i]->Save(cfg, prefix);

	m_midi.Save(cfg, prefix, m_group);
}

void GOrgueManual::Abort()
{
	AllNotesOff();

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->Abort();
}

void GOrgueManual::PreparePlayback()
{
	m_KeyState.resize(m_nb_logical_keys);
	std::fill(m_KeyState.begin(), m_KeyState.end(), 0);
	m_KeyPressed.resize(m_nb_accessible_keys);
	std::fill(m_KeyPressed.begin(), m_KeyPressed.end(), false);
	m_UnisonOff = 0;

	for (unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->PreparePlayback();

	for (unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->PreparePlayback();
}

void GOrgueManual::ProcessMidi(const GOrgueMidiEvent& event)
{
	int key;

	for(unsigned i = 0; i < m_stops.size(); i++)
		m_stops[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_couplers.size(); i++)
		m_couplers[i]->ProcessMidi(event);

	for(unsigned i = 0; i < m_divisionals.size(); i++)
		m_divisionals[i]->ProcessMidi(event);

	switch(m_midi.Match(event, key))
	{
	case MIDI_MATCH_ON:
		Set(key, true);
		break;

	case MIDI_MATCH_OFF:
		Set(key, false);
		break;
		
	case MIDI_MATCH_RESET:
		AllNotesOff();
		break;
		
	default:
		break;
	}
}

void GOrgueManual::Reset()
{
	for (unsigned j = 0; j < GetStopCount(); j++)
                GetStop(j)->Set(false);
	for (unsigned j = 0; j < GetCouplerCount(); j++)
                GetCoupler(j)->Set(false);
	for (unsigned j = 0; j < GetDivisionalCount(); j++)
		GetDivisional(j)->Display(false);
}
