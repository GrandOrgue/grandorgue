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
#include "GOrgueEvent.h"
#include "GOrgueMidi.h"
#include "GOrgueSound.h"
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

const MIDIListenDialog::LISTEN_DIALOG_EVENTS MIDIListenDialog::GetEventFromType(const LISTEN_DIALOG_TYPE type)
{

	static const LISTEN_DIALOG_SETUP_EVENT events0[2] = {
		{wxTRANSLATE("(none)"),            0x0000},
		{wxTRANSLATE("Bx Controller"),     0xB000}
	};

	static const LISTEN_DIALOG_SETUP_EVENT events1[2] = {
		{wxTRANSLATE("(none)"),            0x0000},
		{wxTRANSLATE("9x Note On"),        0x9000}
	};

	static const LISTEN_DIALOG_SETUP_EVENT events2[4] = {
		{wxTRANSLATE("(none)"),            0x0000},
		{wxTRANSLATE("9x Note On"),        0x9000},
		{wxTRANSLATE("Bx Controller"),     0xB000},
		{wxTRANSLATE("Cx Program Change"), 0xC000}
	};

	LISTEN_DIALOG_EVENTS rv;
	switch (type)
	{
	case LSTN_ENCLOSURE:
		rv.count = 2;
		rv.elements = events0;
		break;
	case LSTN_MANUAL:
		rv.count = 2;
		rv.elements = events1;
		break;
	case LSTN_SETTINGSDLG_MEMORY_OR_ORGAN:
	case LSTN_SETTINGSDLG_STOP_CHANGE:
	case LSTN_DRAWSTOP:
	case LSTN_NON_DRAWSTOP_BUTTON:
		rv.count = 4;
		rv.elements = events2;
		break;
	default:
		throw (wxString)_("Invalid event type");
	}

	return rv;

}

wxString MIDIListenDialog::GetEventTitle(int what, const MIDIListenDialog::LISTEN_DIALOG_TYPE type)
{

	what &= 0xF000;

	MIDIListenDialog::LISTEN_DIALOG_EVENTS events = MIDIListenDialog::GetEventFromType(type);

	for (unsigned i = 0; i < events.count; i++)
		if (events.elements[i].event == what)
			return wxGetTranslation(events.elements[i].name);

	return wxEmptyString;

}

unsigned MIDIListenDialog::GetEventChannel(unsigned what)
{

	return (what) ? ((what & 0xF00) >> 8) + 1 : 0;

}

wxString MIDIListenDialog::GetEventChannelString(int what)
{

	int result = GetEventChannel(what);

	if (!result)
		return wxEmptyString;

	wxString retval;
	retval << result;
	return retval;

}

// type 0 : enclosure listening
// type 1 : manual listening
// type 2 : settings dialog - memory next/previous/setter or organ message (which appears to be related to selection of organ to load)
// type 3 : settings dialog - stop change
// type 4 : pushbutton associated with a manual / drawstop listening
// type 5 : pushbutton not associated with a manual listening

MIDIListenDialog::MIDIListenDialog
	(wxWindow* win
	,wxString title
	,const LISTEN_DIALOG_TYPE type
	,const int event_id
	)
	:
	wxDialog(win, wxID_ANY, title),
	m_type(type)
{

	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* sizer = new wxFlexGridSizer(3, 2, 5, 5);
	topSizer->Add(sizer, 0, wxALL, 5);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Event:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

	LISTEN_DIALOG_EVENTS event = GetEventFromType(type);

	m_event = new wxChoice(this, ID_EVENT);
	for (unsigned i = 0; i < event.count; i++)
		m_event->Append(wxGetTranslation(event.elements[i].name));

	sizer->Add(m_event, 1, wxEXPAND);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Channel:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_channel = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, 1, 16);
	box->Add(m_channel, 0);
	box->Add(new wxStaticText(this, wxID_ANY, _("&Data:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_data = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 127);
	box->Add(m_data, 0);

	bool enable_event_channel = (
			(type == LSTN_ENCLOSURE) ||
			(type == LSTN_MANUAL) ||
			(type == LSTN_SETTINGSDLG_MEMORY_OR_ORGAN) ||
			(type == LSTN_SETTINGSDLG_STOP_CHANGE)
		);

	m_event->Enable(enable_event_channel);
	m_channel->Enable(enable_event_channel);
	m_data->Enable(type != LSTN_SETTINGSDLG_STOP_CHANGE);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);

	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->AddSpacer(5);
	SetSizer(topSizer);
	topSizer->Fit(this);

	this->event = 0;
	PutEvent(event_id);

	m_listen->Disable();
	if (g_sound->GetMidi().HasActiveDevice())
		m_listen->Enable();

}

MIDIListenDialog::~MIDIListenDialog()
{
	g_sound->GetMidi().SetListener(NULL);
}

int MIDIListenDialog::GetEvent()
{
	int temp, offset;
	int what = m_event->GetCurrentSelection();
	if (!what)
		return what;
	what = GetEventFromType(m_type).elements[what].event;
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

	if (!m_channel->IsEnabled() && event && m_type != LSTN_NON_DRAWSTOP_BUTTON && (what & 0xF00) >> 8 != m_channel->GetValue() - 1)
		return false;
	if (!m_event->IsEnabled() && event && ((what & 0xF000) != (event & 0xF000)))
		return false;
	if (!m_data->IsEnabled())
		what &= 0xFF00;

	event = what;

	wxString title = GetEventTitle(what, m_type);
	if (m_event->FindString(title) == wxNOT_FOUND)
	{
		if (!what)
			m_event->SetStringSelection(_("(none)"));
		return false;
	}

	m_event->SetStringSelection(title);

	if (m_type != LSTN_NON_DRAWSTOP_BUTTON)
		m_channel->SetValue(GetEventChannel(what));

	int offset = (what & 0xF000) == 0xC000 ? 1 : 0;

	if (m_data->IsEnabled())
	{
		if (m_type == LSTN_MANUAL) // if one of the keyboard
		{
			m_data->SetRange(-11, 11); // data is the note offset
			if ((what&0xFF) < 0x80)
			{
				m_data->SetValue( what & 0x7F );
			} else
			{
				m_data->SetValue( (what & 0xFF) - 140 ); // negative numbers are storedas 128=-12 140=0
			}
		}
		else
		{
			m_data->SetRange(offset, 127 + offset);
			m_data->SetValue((what & 0x7F) + offset);
		}
	}

	bool enable_event_channel = (
			(m_type == LSTN_ENCLOSURE) ||
			(m_type == LSTN_MANUAL) ||
			(m_type == LSTN_SETTINGSDLG_MEMORY_OR_ORGAN) ||
			(m_type == LSTN_SETTINGSDLG_STOP_CHANGE)
		);

	if (!what)
	{
		m_channel->Disable();
		m_data->Disable();
	}
	else
	{
		m_channel->Enable(enable_event_channel);
		m_data->Enable(m_type != LSTN_SETTINGSDLG_STOP_CHANGE);
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
		g_sound->GetMidi().SetListener(GetEventHandler());
	}
	else
	{
		g_sound->GetMidi().SetListener(NULL);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIListenDialog::OnListenMIDI(wxCommandEvent &event)
{
	int what = event.GetInt();
	if (PutEvent(what))
	{
		m_listen->SetValue(false);
		g_sound->GetMidi().SetListener(NULL);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIListenDialog::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(_("MIDI Messages"));
}

