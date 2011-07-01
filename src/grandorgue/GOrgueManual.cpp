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
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "IniFileConfig.h"
#include "MIDIListenDialog.h"
#include "GOrgueMidi.h"
#include "OrganPanel.h"
#include "GOrgueDisplayMetrics.h"

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

GOrgueManual::GOrgueManual() :
	m_manual_number(0),
	m_midi(),
	m_first_accessible_logical_key_nb(0),
	m_nb_logical_keys(0),
	m_first_accessible_key_midi_note_nb(0),
	m_nb_accessible_keys(0),
	m_midi_input_number(0),
	m_nb_stops(0),
	m_nb_couplers(0),
	m_nb_divisionals(0),
	m_nb_tremulants(0),
	m_tremulant_ids(),
	m_name(),
	m_stops(),
	m_couplers(NULL),
	m_divisionals(NULL),
	m_displayed(false),
	m_key_colour_inverted(false),
	m_key_colour_wooden(false)
{

}

void GOrgueManual::Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics, int manualNumber)
{

	m_name                              = cfg.ReadString (group, wxT("Name"), 32);
	m_nb_logical_keys                   = cfg.ReadInteger(group, wxT("NumberOfLogicalKeys"), 1, 192);
	m_first_accessible_logical_key_nb   = cfg.ReadInteger(group, wxT("FirstAccessibleKeyLogicalKeyNumber"), 1, m_nb_logical_keys);
	m_first_accessible_key_midi_note_nb = cfg.ReadInteger(group, wxT("FirstAccessibleKeyMIDINoteNumber"), 0, 127);
	m_nb_accessible_keys                = cfg.ReadInteger(group, wxT("NumberOfAccessibleKeys"), 0, 85);
	m_midi_input_number                 = cfg.ReadInteger(group, wxT("MIDIInputNumber"), 1, 6);
	m_displayed                         = cfg.ReadBoolean(group, wxT("Displayed"));
	m_key_colour_inverted               = cfg.ReadBoolean(group, wxT("DispKeyColourInverted"));
	m_key_colour_wooden                 = cfg.ReadBoolean(group, wxT("DispKeyColourWooden"), false);
	m_nb_stops                          = cfg.ReadInteger(group, wxT("NumberOfStops"), 0, 64);
	m_nb_couplers                       = cfg.ReadInteger(group, wxT("NumberOfCouplers"), 0, 16, false);
	m_nb_divisionals                    = cfg.ReadInteger(group, wxT("NumberOfDivisionals"), 0, 32, false);
	m_nb_tremulants                     = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, 10, false);
	m_manual_number = manualNumber;
	m_display_metrics = displayMetrics;

	wxString buffer;

	for (unsigned i = 0; i < m_nb_stops; i++)
	{
		m_stops.push_back(new GOrgueStop());
		buffer.Printf(wxT("Stop%03d"), i + 1);
		buffer.Printf(wxT("Stop%03d"), cfg.ReadInteger(group, buffer, 1, 448));
		m_stops[i]->m_ManualNumber = m_manual_number;
		m_stops[i]->Load(cfg, buffer, displayMetrics);
	}

	m_couplers = new GOrgueCoupler[m_nb_couplers];
	for (unsigned i = 0; i < m_nb_couplers; i++)
	{
		buffer.Printf(wxT("Coupler%03d"), i + 1);
		buffer.Printf(wxT("Coupler%03d"), cfg.ReadInteger(group, buffer, 1, 64));
		m_couplers[i].Load(cfg, buffer, organfile->GetFirstManualIndex(), organfile->GetManualAndPedalCount(), displayMetrics);
	}

	m_divisionals = new GOrgueDivisional[m_nb_divisionals];
	for (unsigned i = 0; i < m_nb_divisionals; i++)
	{
		buffer.Printf(wxT("Divisional%03d"), i + 1);
		buffer.Printf(wxT("Divisional%03d"), cfg.ReadInteger(group, buffer, 1, 224));
		m_divisionals[i].Load(cfg, buffer, m_manual_number, i, displayMetrics);
	}

	for (unsigned i = 0; i < m_nb_tremulants; i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant_ids[i] = cfg.ReadInteger(group, buffer, 1, organfile->GetTremulantCount());
	}

	for (unsigned i = 0; i < m_nb_stops; i++)
	{
		if (!m_stops[i]->m_auto)
			continue;
		for (unsigned j = 0; j < m_nb_stops; j++)
		{
			if (i == j)
				continue;
			if (m_stops[j]->FirstAccessiblePipeLogicalKeyNumber < m_stops[i]->FirstAccessiblePipeLogicalKeyNumber + m_stops[i]->NumberOfAccessiblePipes &&
					m_stops[j]->FirstAccessiblePipeLogicalKeyNumber + m_stops[j]->NumberOfAccessiblePipes > m_stops[i]->FirstAccessiblePipeLogicalKeyNumber)
			{
				m_stops[i]->m_auto = m_stops[j]->m_auto = false;
				break;
			}
		}
	}
}

void GOrgueManual::Set(int note, bool on, bool pretend, int depth, GOrgueCoupler* prev)
{
	int j;

	// test polyphony?
#if 0
	for (i = 0; i < organfile->m_NumberOfPipes; i++)
		organfile->m_pipe[i]->Set(on);
	return;
#endif

	if (depth > 32)
	{
		::wxLogFatalError(wxT("Infinite recursive coupling detected!"));
		return;
	}

	note -= m_first_accessible_key_midi_note_nb;
	bool outofrange = note < 0 || note >= (int)m_nb_accessible_keys;

	if (!depth && outofrange)
		return;

	if (!outofrange && !pretend)
	{
		if (depth)
		{
			if (!(m_midi[note] >> 1) && !on)
				return;
			m_midi[note] += on ? 2 : -2;
		}
		else
		{
			if ((m_midi[note] & 1) ^ !on)
				return;
			m_midi[note]  = (m_midi[note] & 0xFFFFFFFE) | (on ? 1 : 0);
		}
	}

	bool unisonoff = false;
	for (unsigned i = 0; i < m_nb_couplers; i++)
	{
		if (!m_couplers[i].DefaultToEngaged)
			continue;
		if (m_couplers[i].UnisonOff && (!depth || (prev && prev->CoupleToSubsequentUnisonIntermanualCouplers)))
		{
			unisonoff = true;
			continue;
		}
		j = m_couplers[i].DestinationManual;
		if (
				(!depth)
				||
				(
					(prev)
					&&
					(
						(j == m_manual_number && m_couplers[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntramanualCouplers)
						||
						(j == m_manual_number && m_couplers[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntramanualCouplers)
						||
						(j != m_manual_number && m_couplers[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntermanualCouplers)
						||
						(j != m_manual_number && m_couplers[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntermanualCouplers)
					)
				)
			)
		{
			organfile->GetManual(j)->Set(note + m_first_accessible_key_midi_note_nb + m_couplers[i].DestinationKeyshift, on, false, depth + 1, m_couplers + i);
		}
	}

	if (!unisonoff)
	{
		for (unsigned i = 0; i < m_nb_stops; i++)
		{
			if (!m_stops[i]->DefaultToEngaged)
				continue;
			j = note - (m_stops[i]->FirstAccessiblePipeLogicalKeyNumber - 1);
			j += (m_first_accessible_logical_key_nb - 1); // Correction to take FirstAccessibleKeyLogicalKeyNumber of the manual into account
			if (j < 0 || j >= m_stops[i]->NumberOfAccessiblePipes)
				continue;
			j += m_stops[i]->FirstAccessiblePipeLogicalPipeNumber - 1;

			organfile->GetPipe(m_stops[i]->pipe[j])->Set(on);
		}
	}

	if (!outofrange)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_manual_number);
		event.SetExtraLong(note);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}


void GOrgueManual::MIDI(void)
{

	int index = m_midi_input_number + 7;

	MIDIListenDialog dlg
		(::wxGetApp().frame
		,GOrgueMidi::GetMidiEventTitle(index)
		,MIDIListenDialog::LSTN_MANUAL
		,g_sound->GetMidi().GetManualMidiEvent(m_midi_input_number)
		);

	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfigBase::Get()->Write
			(wxString(wxT("MIDI/")) + GOrgueMidi::GetMidiEventTitle(index)
			,dlg.GetEvent()
			);
		g_sound->ResetSound(organfile);
	}

}


template<class T>
struct delete_functor
{
	void operator()(T* p)
	{
		delete p;
	}
};

GOrgueManual::~GOrgueManual(void)
{
	std::for_each(m_stops.begin(), m_stops.end(), delete_functor<GOrgueStop>());
	if (m_couplers)
		delete[] m_couplers;
	if (m_divisionals)
		delete[] m_divisionals;
}

int GOrgueManual::GetMIDIInputNumber()
{
	return m_midi_input_number;
}

int GOrgueManual::GetLogicalKeyCount()
{
	return m_nb_logical_keys;
}

int GOrgueManual::GetNumberOfAccessibleKeys()
{
	return m_nb_accessible_keys;
}

/* TODO: I suspect this could be made private or into something better... */
int GOrgueManual::GetFirstAccessibleKeyMIDINoteNumber()
{
	return m_first_accessible_key_midi_note_nb;
}

int GOrgueManual::GetStopCount()
{
	return m_nb_stops;
}

GOrgueStop* GOrgueManual::GetStop(unsigned index)
{
	assert(index < m_nb_stops);
	return m_stops[index];
}

int GOrgueManual::GetCouplerCount()
{
	return m_nb_couplers;
}

GOrgueCoupler* GOrgueManual::GetCoupler(unsigned index)
{
	assert(index < m_nb_couplers);
	return &m_couplers[index];
}

int GOrgueManual::GetDivisionalCount()
{
	return m_nb_divisionals;
}

GOrgueDivisional* GOrgueManual::GetDivisional(unsigned index)
{
	assert(index < m_nb_divisionals);
	return &m_divisionals[index];
}

int GOrgueManual::GetTremulantCount()
{
	return m_nb_tremulants;
}

GOrgueTremulant* GOrgueManual::GetTremulant(unsigned index)
{
	assert(index < m_nb_tremulants);
	return organfile->GetTremulant(m_tremulant_ids[index] - 1);
}

void GOrgueManual::AllNotesOff()
{

	/* TODO: I'm not sure if these are allowed to be merged into one loop. */
	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
        m_midi[j] = 0;

	for (unsigned j = 0; j < m_nb_accessible_keys; j++)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_manual_number);
		event.SetExtraLong(j);
		::wxGetApp().frame->AddPendingEvent(event);
	}

}

/* TODO: figure out what this thing does and document it */
void GOrgueManual::MIDIPretend(bool on)
{
	for (unsigned j = 0; j < m_nb_logical_keys; j++)
		if (m_midi[j] & 1)
			Set(j + m_first_accessible_key_midi_note_nb, on, true);
}

bool GOrgueManual::IsKeyDown(int midiNoteNumber)
{
	if (midiNoteNumber < m_first_accessible_key_midi_note_nb)
		return false;
	if (midiNoteNumber > m_first_accessible_key_midi_note_nb + (int)m_nb_logical_keys - 1)
		return false;
	return m_midi[midiNoteNumber - m_first_accessible_key_midi_note_nb];
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

	const GOrgueDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

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
		if (key_midi_nb > m_first_accessible_key_midi_note_nb && j && j != 5)
			z |= 2;
		if (key_midi_nb < m_first_accessible_key_midi_note_nb + (int)m_nb_accessible_keys - 1 && j != 4 && j != 11)
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

	const GOrgueDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

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

	const GOrgueDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

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

	const GOrgueDisplayMetrics::MANUAL_RENDER_INFO &mri = m_display_metrics->GetManualRenderInfo(m_manual_number);

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

	if (m_manual_number < organfile->GetFirstManualIndex())
		return;

	wxFont font = m_display_metrics->GetControlLabelFont();
	for (unsigned j = 0; j < m_nb_stops; j++)
	{
		if (m_stops[j]->Displayed)
		{
			font.SetPointSize(m_stops[j]->DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_stops[j]->Name, 51);
		}
	}

	for (unsigned j = 0; j < m_nb_couplers; j++)
	{
		if (m_couplers[j].Displayed)
		{
			font.SetPointSize(m_couplers[j].DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_couplers[j].Name, 51);
		}
	}

	for (unsigned j = 0; j < m_nb_divisionals; j++)
	{
		if (m_divisionals[j].Displayed)
		{
			font.SetPointSize(m_divisionals[j].DispLabelFontSize);
			dc.SetFont(font);
			OrganPanel::WrapText(dc, m_divisionals[j].Name, 28);
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
