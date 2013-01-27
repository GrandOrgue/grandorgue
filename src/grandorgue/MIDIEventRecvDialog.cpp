/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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
 */

#include "MIDIEventRecvDialog.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

IMPLEMENT_CLASS(MIDIEventRecvDialog, wxPanel)

BEGIN_EVENT_TABLE(MIDIEventRecvDialog, wxPanel)
	EVT_TOGGLEBUTTON(ID_LISTEN, MIDIEventRecvDialog::OnListenClick)
	EVT_BUTTON(ID_EVENT_NEW, MIDIEventRecvDialog::OnNewClick)
	EVT_BUTTON(ID_EVENT_DELETE, MIDIEventRecvDialog::OnDeleteClick)
	EVT_CHOICE(ID_EVENT_NO, MIDIEventRecvDialog::OnEventChange)
	EVT_MIDI(MIDIEventRecvDialog::OnMidiEvent)
END_EVENT_TABLE()


MIDIEventRecvDialog::MIDIEventRecvDialog (wxWindow* parent, const GOrgueMidiReceiver& event):
	wxPanel(parent, wxID_ANY),
	m_midi(event)
{

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* sizer = new wxFlexGridSizer(5, 2, 5, 5);
	topSizer->Add(sizer, 0, wxALL, 6);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("Event-&No")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

	wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_eventno = new wxChoice(this, ID_EVENT_NO);
	box->Add(m_eventno, 1, wxEXPAND);
	m_new = new wxButton(this, ID_EVENT_NEW, _("New"));
	m_delete = new wxButton(this, ID_EVENT_DELETE, _("Delete"));
	box->Add(m_new, 0);
	box->Add(m_delete, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Device:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

	m_device = new wxChoice(this, ID_EVENT);
	sizer->Add(m_device, 1, wxEXPAND);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Event:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

	m_eventtype = new wxChoice(this, ID_EVENT);
	sizer->Add(m_eventtype, 1, wxEXPAND);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Channel:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_channel = new wxChoice(this, ID_CHANNEL);
	box->Add(m_channel, 0);
	if (m_midi.GetType() == MIDI_RECV_MANUAL)
		box->Add(new wxStaticText(this, wxID_ANY, _("&Transpose:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	else
		box->Add(new wxStaticText(this, wxID_ANY, _("&Data:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_data = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 127);
	box->Add(m_data, 0);

	if (m_midi.GetType() == MIDI_RECV_MANUAL)
		m_data->SetRange(-35, 35);
	else
		m_data->SetRange(0, 0x200000);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Lowest key:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowKey = new wxSpinCtrl(this, ID_LOW_KEY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowKey, 0);
	box->Add(new wxStaticText(this, wxID_ANY, _("&Highest key:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_HighKey = new wxSpinCtrl(this, ID_HIGH_KEY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighKey, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("L&owest velocity:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowVelocity = new wxSpinCtrl(this, ID_LOW_VELOCITY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowVelocity, 0);
	box->Add(new wxStaticText(this, wxID_ANY, _("H&ighest velocity:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_HighVelocity = new wxSpinCtrl(this, ID_HIGH_VELOCITY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighVelocity, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);
	SetSizer(topSizer);

	m_device->Append(_("Any device"));

	std::vector<wxString> device_names = m_midi.GetSettings().GetMidiInDeviceList();
	for(std::vector<wxString>::iterator it = device_names.begin(); it != device_names.end(); it++)
		m_device->Append(*it);

	m_channel->Append(_("Any channel"));
	for(unsigned int i = 1 ; i <= 16; i++)
		m_channel->Append(wxString::Format(wxT("%d"), i));;

	m_eventtype->Append(_("(none)"), (void*)MIDI_M_NONE);
	if (m_midi.GetType() != MIDI_RECV_ENCLOSURE)
		m_eventtype->Append(_("9x Note"), (void*)MIDI_M_NOTE);
	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
	{
		m_eventtype->Append(_("9x Note On Toogle"), (void*)MIDI_M_NOTE_ON);
		m_eventtype->Append(_("9x Note Off Toggle"), (void*)MIDI_M_NOTE_OFF);
	}
	if (m_midi.GetType() != MIDI_RECV_MANUAL)
		m_eventtype->Append(_("Bx Controller"), (void*)MIDI_M_CTRL_CHANGE);
	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
	{
		m_eventtype->Append(_("Bx Controller On Toogle"), (void*)MIDI_M_CTRL_CHANGE_ON);
		m_eventtype->Append(_("Bx Controller Off Toogle"), (void*)MIDI_M_CTRL_CHANGE_OFF);
	}
	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
		m_eventtype->Append(_("Cx Program Change"), (void*)MIDI_M_PGM_CHANGE);
	if (m_midi.GetType() == MIDI_RECV_MANUAL)
	{
		m_eventtype->Append(_("9x Note without Velocity"), (void*)MIDI_M_NOTE_NO_VELOCITY);
		m_eventtype->Append(_("9x Note short octave at low key"), (void*)MIDI_M_NOTE_SHORT_OCTAVE);
	}

	if (m_midi.GetType() == MIDI_RECV_MANUAL || m_midi.GetType() == MIDI_RECV_ENCLOSURE)
	{
		m_LowVelocity->Enable();
		m_HighVelocity->Enable();
	}
	else
	{
		m_LowVelocity->Disable();
		m_HighVelocity->Disable();
	}

	if (m_midi.GetType() == MIDI_RECV_MANUAL)
	{
		m_LowKey->Enable();
		m_HighKey->Enable();
	}
	else
	{
		m_LowKey->Disable();
		m_HighKey->Disable();
	}


	m_current = 0;
	if (!m_midi.GetEventCount())
		m_midi.AddNewEvent();

	LoadEvent();

	topSizer->Fit(this);
}

MIDIEventRecvDialog::~MIDIEventRecvDialog()
{
	m_midi.GetOrganfile()->SetMidiListener(NULL);
}

const GOrgueMidiReceiver& MIDIEventRecvDialog::GetResult()
{
	StoreEvent();

	bool empty_event;
	do
	{
		empty_event = false;
		for(unsigned i = 0; i < m_midi.GetEventCount(); i++)
			if (m_midi.GetEvent(i).type == MIDI_M_NONE)
			{
				m_midi.DeleteEvent(i);
				empty_event = true;
			}
	}
	while(empty_event);

	return m_midi;
}

void MIDIEventRecvDialog::LoadEvent()
{
	m_eventno->Clear();
	for(unsigned i = 0; i < m_midi.GetEventCount(); i++)
		{
			wxString buffer;
			wxString device;
			if (m_midi.GetEvent(i).device.IsEmpty())
				device =  _("Any device");
			else
				device = m_midi.GetEvent(i).device;
			buffer.Printf(_("%d (%s)"), i + 1, device.c_str());
			m_eventno->Append(buffer);
		}
	m_eventno->Select(m_current);
	if (m_midi.GetEventCount() > 1)
		m_delete->Enable();
	else
		m_delete->Disable();

	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);

	m_eventtype->SetSelection(0);
	for(unsigned i = 0; i < m_eventtype->GetCount(); i++)
		if ((void*)e.type == m_eventtype->GetClientData(i))
			m_eventtype->SetSelection(i);

	m_device->SetSelection(0);
	for(unsigned i = 1; i < m_device->GetCount(); i++)
		if (e.device == m_device->GetString(i))
			m_device->SetSelection(i);

	m_channel->SetSelection(0);
	if (e.channel == -1)
		m_channel->SetSelection(0);
	else
		m_channel->SetSelection(e.channel);

	m_data->SetValue(e.key);
	m_LowKey->SetValue(e.low_key);
	m_HighKey->SetValue(e.high_key);
	m_LowVelocity->SetValue(e.low_velocity);
	m_HighVelocity->SetValue(e.high_velocity);
}

void MIDIEventRecvDialog::StoreEvent()
{
	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	if (m_device->GetSelection() == 0)
		e.device = wxT("");
	else
		e.device = m_device->GetStringSelection();

	e.type = (midi_match_message_type)(intptr_t) m_eventtype->GetClientData(m_eventtype->GetSelection());
	if (m_channel->GetSelection() == 0)
		e.channel = -1;
	else
		e.channel = m_channel->GetSelection();

	e.key = m_data->GetValue();
	e.low_key = m_LowKey->GetValue();
	e.high_key = m_HighKey->GetValue();
	e.low_velocity = m_LowVelocity->GetValue();
	e.high_velocity = m_HighVelocity->GetValue();
}

void MIDIEventRecvDialog::OnNewClick(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_midi.AddNewEvent();
	LoadEvent();
}

void MIDIEventRecvDialog::OnDeleteClick(wxCommandEvent& event)
{
	m_midi.DeleteEvent(m_current);
	m_current = 0;
	LoadEvent();
}

void MIDIEventRecvDialog::OnEventChange(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_eventno->GetSelection();
	LoadEvent();
}

void MIDIEventRecvDialog::OnListenClick(wxCommandEvent& event)
{
	if (m_listen->GetValue())
	{
		this->SetCursor(wxCursor(wxCURSOR_WAIT));
		m_midi.GetOrganfile()->SetMidiListener(GetEventHandler());
	}
	else
	{
		m_midi.GetOrganfile()->SetMidiListener(NULL);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIEventRecvDialog::OnMidiEvent(GOrgueMidiEvent& event)
{
	if (event.GetMidiType() == MIDI_NONE)
		return;

	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	e.type = (midi_match_message_type)event.GetMidiType();
	e.device = event.GetDevice();
	e.channel = event.GetChannel();
	if (m_midi.GetType() != MIDI_RECV_MANUAL)
		e.key = event.GetKey();
	e.low_key = 0;
	e.high_key = 127;
	e.low_velocity = m_midi.GetType() == MIDI_RECV_ENCLOSURE ? 0 : 1;
	e.high_velocity = 127;

	LoadEvent();

	m_listen->SetValue(false);
	m_midi.GetOrganfile()->SetMidiListener(NULL);
	this->SetCursor(wxCursor(wxCURSOR_ARROW));
}
