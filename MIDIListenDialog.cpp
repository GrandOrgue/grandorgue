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

#include <wx/statline.h>
#include "MIDIListenDialog.h"
#include "GOrgueSound.h"
#include "MIDIEvents.h"
#include "GrandOrgue.h"

/* TODO: This should not be... */
extern GOrgueSound* g_sound;

IMPLEMENT_CLASS(MIDIListenDialog, wxDialog)

BEGIN_EVENT_TABLE(MIDIListenDialog, wxDialog)
	EVT_CHOICE(ID_EVENT, MIDIListenDialog::OnEvent)
	EVT_TOGGLEBUTTON(ID_LISTEN, MIDIListenDialog::OnListenClick)
	EVT_COMMAND(0, wxEVT_LISTENING, MIDIListenDialog::OnListenMIDI)
	EVT_BUTTON(wxID_HELP, MIDIListenDialog::OnHelp)
END_EVENT_TABLE()

wxString GetEventTitle(int what, int type)
{
	what &= 0xF000;
	for (int i = 0; i < g_MIDIEvents[type].count; i++)
		if (g_MIDIEvents[type].events[i] == what)
			return _(g_MIDIEvents[type].names[i]);

	return wxEmptyString;
}

int GetEventChannel(int what)
{
	if (!what)
		return 0;
	return ((what & 0xF00) >> 8) + 1;
}

wxString GetEventChannelString(int what)
{
	int result = GetEventChannel(what);
	if (!result)
		return wxEmptyString;
	wxString retval;
	retval << result;
	return retval;
}

MIDIListenDialog::MIDIListenDialog(wxWindow* win, wxString title, int what, int type) : wxDialog(win, wxID_ANY, title)
{
	this->type = type;
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(3, 2, 5, 5);
	topSizer->Add(sizer, 0, wxALL, 5);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Event:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxArrayString choices(g_MIDIEvents[type].count, g_MIDIEvents[type].names);
	m_event = new wxChoice(this, ID_EVENT);
	for (int i = 0; i < g_MIDIEvents[type].count; i++)
        m_event->Append(_(g_MIDIEvents[type].names[i]));
	sizer->Add(m_event, 1, wxEXPAND);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Channel:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_channel = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 16);
	box->Add(m_channel, 0);
	box->Add(new wxStaticText(this, wxID_ANY, _("&Data:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_data = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 127);
	box->Add(m_data, 0);

	m_event->Enable(type < 4);
	m_channel->Enable(type < 4);
	m_data->Enable(type != 3);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);

	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);
    topSizer->AddSpacer(5);
    SetSizer(topSizer);
	topSizer->Fit(this);

	event = 0;
	PutEvent(what);

	m_listen->Disable();
	for (int i = 0; i < g_sound->n_midiDevices; i++)
		if (g_sound->b_midiDevices[i])
			m_listen->Enable();
}

MIDIListenDialog::~MIDIListenDialog()
{
	g_sound->b_listening = false;
}

int MIDIListenDialog::GetEvent()
{
  int temp, offset;
  int what = m_event->GetCurrentSelection();
	if (!what)
		return what;
	what = g_MIDIEvents[type].events[what];
	what |= ((m_channel->GetValue() - 1) & 0xF) << 8;
    temp = m_data->GetValue();
    if ( temp )
    {
        if (temp < 0) temp = 140 + temp; // negative numbers are coded as 128=-12 140=0
        offset = (what & 0xF000) == 0xC000 ? 1 : 0;
        what |= temp - offset;
    }
	return what;
}

bool MIDIListenDialog::PutEvent(int what)
{
	if (!m_channel->IsEnabled() && event && type != 5 && (what & 0xF00) >> 8 != m_channel->GetValue() - 1)
		return false;
	if (!m_event->IsEnabled() && event && ((what & 0xF000) != (event & 0xF000)))
		return false;
	if (!m_data->IsEnabled())
		what &= 0xFF00;

	event = what;

	wxString title = GetEventTitle(what, type);
	if (m_event->FindString(title) == wxNOT_FOUND)
	{
		if (!what)
			m_event->SetStringSelection(_("(none)"));
		return false;
	}

	m_event->SetStringSelection(title);

	if (type != 5)
		m_channel->SetValue(GetEventChannel(what));
	int offset = (what & 0xF000) == 0xC000 ? 1 : 0;
	if (m_data->IsEnabled())
	{
	    if (type==1) // if one of the keyboard
	    {
	        m_data->SetRange(-11, 11); // data is the note offset
	        if ((what&0xFF) < 0x80)
	        {
	            m_data->SetValue( what & 0x7F );
	        } else
	        {
	            m_data->SetValue( (what & 0xFF) - 140 ); // negative numbers are storedas 128=-12 140=0
            }
	    }else
	    {
	        m_data->SetRange(offset, 127 + offset);
	        m_data->SetValue((what & 0x7F) + offset);
	    }
	}

	if (!what)
	{
		m_channel->Disable();
		m_data->Disable();
	}
	else
	{
		m_channel->Enable(type < 4);
		m_data->Enable(type != 3);
	}

	return true;
}

void MIDIListenDialog::OnEvent(wxCommandEvent& event)
{
	PutEvent(GetEvent());
}

void MIDIListenDialog::OnListenClick(wxCommandEvent &event)
{
	if (m_listen->GetValue())
	{
		this->SetCursor(wxCursor(wxCURSOR_WAIT));
		g_sound->listen_evthandler = GetEventHandler();
		g_sound->b_listening = true;
	}
	else
	{
		g_sound->b_listening = false;
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIListenDialog::OnListenMIDI(wxCommandEvent &event)
{
	int what = event.GetInt();
	if (PutEvent(what))
	{
		m_listen->SetValue(g_sound->b_listening = false);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIListenDialog::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(_("MIDI Messages"));
}

