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

#include "GOrguePushbutton.h"
#include "IniFileConfig.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueSound.h"
#include "MIDIEventDialog.h"
#include "GOrgueDisplayMetrics.h"
#include "GOrgueMidi.h"

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

GOrguePushbutton::GOrguePushbutton() :
	GOrgueControl(),
	m_midi(MIDI_RECV_BUTTON),
	m_ManualNumber(0),
	DispButtonRow(0),
	DispButtonCol(0),
	DispImageNum(0)
{
}

void GOrguePushbutton::MIDI(void)
{
	MIDIEventDialog dlg (::wxGetApp().frame, _("Midi-Settings for Pushbutton - ")+Name ,m_midi);

	if (dlg.ShowModal() == wxID_OK)
	{
		m_midi = dlg.GetResult();
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
}

bool GOrguePushbutton::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
	int x, y;
	if (!Displayed)
		return false;

	DisplayMetrics->GetPushbuttonBlitPosition(DispButtonRow, DispButtonCol, &x, &y);

	if (!DispKeyLabelOnLeft)
		x -= 13;

	if (!dc)
		return !(xx < x || xx > x + 30 || yy < y || yy > y + 30 || (x + 15 - xx) * (x + 15 - xx) + (y + 15 - yy) * (y + 15 - yy) > 225);

	wxMemoryDC mdc;
	wxRect rect(x + 1, y + 1, 31 - 1, 30 - 1);
	wxBitmap* bmp = organfile->GetImage(DispImageNum + 4);
	dc->DrawBitmap(*bmp, x, y, true);
	dc->SetTextForeground(DispLabelColour);
	wxFont font = DisplayMetrics->GetControlLabelFont();
	font.SetPointSize(DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	if (dc2)
		dc2->Blit(x, y, 31, 30, dc, x, y);
	return false;

}

void GOrguePushbutton::Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics)
{
	DisplayMetrics = displayMetrics;
	DispButtonRow = cfg.ReadInteger(group, wxT("DispButtonRow"), 0, 99 + displayMetrics->NumberOfExtraButtonRows());
	DispButtonCol = cfg.ReadInteger(group, wxT("DispButtonCol"), 1, displayMetrics->NumberOfButtonCols());
	DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2);
	DispImageNum--;
	m_midi.Load(cfg, group);
	GOrgueControl::Load(cfg, group);
}

void GOrguePushbutton::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
	m_midi.Save(cfg, prefix, group);
}

void GOrguePushbutton::Display(bool onoff)
{
	int on = (onoff ? 2 : 0);
	if ((DispImageNum & 2) != on)
	{
		DispImageNum = (DispImageNum & 1) | on;
		wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
		event.SetClientData(this);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}

void GOrguePushbutton::ProcessMidi(const GOrgueMidiEvent& event)
{
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
	case MIDI_MATCH_ON:
		Push();
		break;

	default:
		break;
	}
}
