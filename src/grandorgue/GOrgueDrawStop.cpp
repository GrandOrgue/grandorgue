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
#include "GOrgueDisplayMetrics.h"
#include "GOrguePiston.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "MIDIListenDialog.h"
#include "GOrgueMidi.h"

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

GOrgueDrawstop::GOrgueDrawstop() :
	GOrgueControl(),
	DefaultToEngaged(false),
	DisplayInInvertedState(false),
	DispDrawstopRow(0),
	DispDrawstopCol(0),
	DispImageNum(0),
	StopControlMIDIKeyNumber(0)
{

}

GOrgueDrawstop::~GOrgueDrawstop()
{

}

void GOrgueDrawstop::Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics)
{
	DisplayMetrics = displayMetrics;
	DispDrawstopRow = cfg.ReadInteger(group, wxT("DispDrawstopRow"), 1, 99 + DisplayMetrics->NumberOfExtraDrawstopRowsToDisplay());
	DispDrawstopCol = cfg.ReadInteger(group, wxT("DispDrawstopCol"), 1, DispDrawstopRow > 99 ? DisplayMetrics->NumberOfExtraDrawstopColsToDisplay() : DisplayMetrics->NumberOfDrawstopColsToDisplay());
	DefaultToEngaged = cfg.ReadBoolean(group, wxT("DefaultToEngaged"));
	DisplayInInvertedState = cfg.ReadBoolean(group, wxT("DisplayInInvertedState"));
	DispImageNum = cfg.ReadInteger(group, wxT("DispImageNum"), 1, 2);
	StopControlMIDIKeyNumber = cfg.ReadInteger(group, wxT("StopControlMIDIKeyNumber"), 0, 127, false);
	GOrgueControl::Load(cfg, group);
}

void GOrgueDrawstop::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
	cfg.SaveHelper(prefix, group, wxT("DefaultToEngaged"), DefaultToEngaged ? wxT("Y") : wxT("N"));
	cfg.SaveHelper(prefix, group, wxT("StopControlMIDIKeyNumber"), StopControlMIDIKeyNumber);
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
	wxBitmap* bmp = organfile->GetImage(((DispImageNum - 1) << 1) + (DisplayInInvertedState ^ DefaultToEngaged ? 1 : 0));
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
	Set(DefaultToEngaged ^ true);
};

void GOrgueDrawstop::MIDI(void)
{

	MIDIListenDialog dlg
		(::wxGetApp().frame
		,_("Drawstop Trigger")
		,MIDIListenDialog::LSTN_DRAWSTOP
		,g_sound->GetMidi().GetStopMidiEvent() | StopControlMIDIKeyNumber
		);

	if (dlg.ShowModal() == wxID_OK)
	{

		StopControlMIDIKeyNumber = dlg.GetEvent() & 0x7F;
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);

	}

}

void GOrgueDrawstop::Set(bool on)
{
	if (DefaultToEngaged == on)
		return;
	DefaultToEngaged = on;
	wxCommandEvent event(wxEVT_DRAWSTOP, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
	for (unsigned i = 0; i < organfile->GetNumberOfReversiblePistons(); i++)
	{
		if (organfile->GetPiston(i)->drawstop == this)
		{
			organfile->GetPiston(i)->DispImageNum = (organfile->GetPiston(i)->DispImageNum & 1) | (on ^ DisplayInInvertedState ? 2 : 0);
			wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
			/*event.SetClientData(organfile->m_piston + i);*/ /* TODO: the equivilent of this may be wrong */
			event.SetClientData(organfile->GetPiston(i));
			::wxGetApp().frame->AddPendingEvent(event);
	    }
	}
	GOrgueLCD_WriteLineTwo(Name, 2000);
}

void GOrgueDrawstop::ProcessMidi(const GOrgueMidiEvent& event)
{
}
