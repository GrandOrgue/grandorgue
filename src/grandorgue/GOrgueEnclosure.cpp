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

#include <wx/app.h>
#include "GOrgueEnclosure.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "MIDIEventDialog.h"
#include "GOrgueDisplayMetrics.h"
#include "IniFileConfig.h"
#include "GOrgueMidi.h"

GOrgueEnclosure::GOrgueEnclosure(GrandOrgueFile* organfile) :
	m_group(wxT("---")),
	m_midi(organfile, MIDI_RECV_ENCLOSURE),
	m_organfile(organfile),
	AmpMinimumLevel(0),
	MIDIInputNumber(0),
	MIDIValue(0),
	Name()
{

}

bool GOrgueEnclosure::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{

	int enclosure_x = DisplayMetrics->GetEnclosureX(this);
	int enclosure_y = DisplayMetrics->GetEnclosureY();

	if (!dc)
	{
		wxRect rect(enclosure_x, enclosure_y, 46, 61);
		return rect.Contains(xx, yy);
	}

	dc->SetBrush(*wxBLACK_BRUSH);
	dc->DrawRectangle(enclosure_x, enclosure_y + 13, 46, 44);
	int dx = 1 + ( 3 * MIDIValue) / 127;
	int dy = 1 + (13 * MIDIValue) / 127;
	wxPoint points[4];
	points[0].x = enclosure_x +  7 + dx;
	points[1].x = enclosure_x + 38 - dx;
	points[2].x = enclosure_x + 38 + dx;
	points[3].x = enclosure_x +  7 - dx;
	points[0].y = points[1].y = enclosure_y + 13 + dy;
	points[2].y = points[3].y = enclosure_y + 56 - dy;
	dc->SetBrush(::wxGetApp().frame->m_pedalBrush);
	dc->DrawPolygon(4, points);

	if (dc2)
	{
		dc2->Blit
			(enclosure_x
			,enclosure_y + 13
			,46
			,44
			,dc
			,enclosure_x
			,enclosure_y + 13
			);
	}

	return false;

}

void GOrgueEnclosure::Load(IniFileConfig& cfg, const unsigned enclosure_nb, GOrgueDisplayMetrics* displayMetrics)
{
	m_group.Printf(wxT("Enclosure%03u"), enclosure_nb + 1);
	m_enclosure_nb = enclosure_nb;
	DisplayMetrics = displayMetrics;
	Name = cfg.ReadString(m_group, wxT("Name"), 64);
	AmpMinimumLevel = cfg.ReadInteger(m_group, wxT("AmpMinimumLevel"), 0, 100);
	MIDIInputNumber = cfg.ReadInteger(m_group, wxT("MIDIInputNumber"), 1, 6);
	Set(127);	// default to full volume until we receive any messages
	m_midi.SetManual(enclosure_nb);
	m_midi.Load(cfg, m_group);
}

void GOrgueEnclosure::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
}

void GOrgueEnclosure::Set(int n)
{
	if (n < 0)
		n = 0;
	if (n > 127)
		n = 127;
	MIDIValue = n;
	wxCommandEvent event(wxEVT_ENCLOSURE, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
}

void GOrgueEnclosure::MIDI(void)
{
	MIDIEventDialog dlg (::wxGetApp().frame, _("Midi-Settings for Enclosure - ")+Name ,m_midi);

	if (dlg.ShowModal() == wxID_OK)
	{
		m_midi = dlg.GetResult();
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}

}

int GOrgueEnclosure::GetMIDIInputNumber()
{

	return MIDIInputNumber;

}

float GOrgueEnclosure::GetAttenuation()
{

	static const float scale = 1.0 / 12700.0;
	return (float)(MIDIValue * (100 - AmpMinimumLevel) + 127 * AmpMinimumLevel) * scale;

}

void GOrgueEnclosure::DrawLabel(wxDC& dc)
{

	int enclosure_x = DisplayMetrics->GetEnclosureX(this);
	int enclosure_y = DisplayMetrics->GetEnclosureY();

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);

	wxRect rect(enclosure_x, enclosure_y, 46, 61);
	dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	wxFont font = *wxNORMAL_FONT;
	font.SetPointSize(7);
	dc.SetFont(font);
	dc.SetTextForeground(*wxWHITE);

	dc.DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL);

}

void GOrgueEnclosure::Scroll(bool scroll_up)
{

	Set(MIDIValue + (scroll_up ? 16 : -16));

}

bool GOrgueEnclosure::IsEnclosure(const unsigned nb) const
{

	return (m_enclosure_nb == nb);

}

void GOrgueEnclosure::ProcessMidi(const GOrgueMidiEvent& event)
{
	int value;
	if (m_midi.Match(event, value) == MIDI_MATCH_CHANGE)
		Set(value);
}
