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

#include "GOrgueDrawStop.h"

#include <wx/docview.h>
#include "GOGUIDisplayMetrics.h"
#include "GOrguePiston.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "MIDIEventDialog.h"
#include "GOrgueMidi.h"

GOrgueDrawstop::GOrgueDrawstop(GrandOrgueFile* organfile) :
	GOrgueControl(),
	m_midi(organfile, MIDI_RECV_DRAWSTOP),
	m_organfile(organfile),
	m_DefaultToEngaged(false),
	DisplayInInvertedState(false),
	DispDrawstopRow(0),
	DispDrawstopCol(0),
	DispImageNum(0)
{

}

GOrgueDrawstop::~GOrgueDrawstop()
{

}

void GOrgueDrawstop::Load(IniFileConfig& cfg, wxString group, GOGUIDisplayMetrics* displayMetrics)
{
	DisplayMetrics = displayMetrics;
	DispDrawstopRow = cfg.ReadInteger(group, wxT("DispDrawstopRow"), 1, 99 + DisplayMetrics->NumberOfExtraDrawstopRowsToDisplay());
	DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, DispDrawstopRow > 99 ? DisplayMetrics->NumberOfExtraDrawstopColsToDisplay() : DisplayMetrics->NumberOfDrawstopColsToDisplay());
	m_DefaultToEngaged = cfg.ReadBoolean(group, wxT("DefaultToEngaged"));
	DisplayInInvertedState = cfg.ReadBoolean(group, wxT("DisplayInInvertedState"));
	DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2);
	m_midi.Load(cfg, group);
	GOrgueControl::Load(cfg, group);
}

void GOrgueDrawstop::Save(IniFileConfig& cfg, bool prefix)
{
	m_midi.Save(cfg, prefix, m_group);
	cfg.SaveHelper(prefix, m_group, wxT("DefaultToEngaged"), m_DefaultToEngaged ? wxT("Y") : wxT("N"));
	GOrgueControl::Save(cfg, prefix);
}

bool GOrgueDrawstop::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
	int x, y;
	if (!Displayed)
		return false;

	DisplayMetrics->GetDrawstopBlitPosition(DispDrawstopRow, DispDrawstopCol, &x, &y);

	if (!dc)
		return !(xx < x || xx > x + 64 || yy < y || yy > y + 64 || (x + 32 - xx) * (x + 32 - xx) + (y + 32 - yy) * (y + 32 - yy) > 1024);

	wxRect rect(x, y + 1, 65, 65 - 1);
	wxBitmap* bmp = m_organfile->GetImage(((DispImageNum - 1) << 1) + (DisplayInInvertedState ^ m_DefaultToEngaged ? 1 : 0));
	dc->DrawBitmap(*bmp, x, y, true);
	dc->SetTextForeground(DispLabelColour);
	wxFont font = DisplayMetrics->GetControlLabelFont();
	font.SetPointSize(DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

	if (dc2)
		dc2->Blit(x, y, 65, 65, dc, x, y);
	return false;

}

void GOrgueDrawstop::Push()
{
	Set(m_DefaultToEngaged ^ true);
};

void GOrgueDrawstop::MIDI(void)
{
	MIDIEventDialog dlg (::wxGetApp().frame, _("Midi-Settings for Drawstop - ")+Name ,m_midi);

	if (dlg.ShowModal() == wxID_OK)
	{
		m_midi = dlg.GetResult();
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
}

void GOrgueDrawstop::Set(bool on)
{
	if (m_DefaultToEngaged == on)
		return;
	m_DefaultToEngaged = on;
	wxCommandEvent event(wxEVT_DRAWSTOP, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
	for (unsigned i = 0; i < m_organfile->GetNumberOfReversiblePistons(); i++)
	{
		if (m_organfile->GetPiston(i)->drawstop == this)
		{
			m_organfile->GetPiston(i)->DispImageNum = (m_organfile->GetPiston(i)->DispImageNum & 1) | (on ^ DisplayInInvertedState ? 2 : 0);
			wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
			/*event.SetClientData(m_organfile->m_piston + i);*/ /* TODO: the equivilent of this may be wrong */
			event.SetClientData(m_organfile->GetPiston(i));
			::wxGetApp().frame->AddPendingEvent(event);
	    }
	}
	GOrgueLCD_WriteLineTwo(Name, 2000);
}

void GOrgueDrawstop::ProcessMidi(const GOrgueMidiEvent& event)
{
	switch(m_midi.Match(event))
	{
	case MIDI_MATCH_CHANGE:
		Push();
		break;

	case MIDI_MATCH_ON:
		Set(true);
		break;

	case MIDI_MATCH_OFF:
		Set(false);
		break;

	default:
		break;
	}
}

bool GOrgueDrawstop::IsEngaged() const
{
	return m_DefaultToEngaged;
}
