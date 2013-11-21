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

BEGIN_EVENT_TABLE(MIDIEventRecvDialog, wxPanel)
	EVT_TOGGLEBUTTON(ID_LISTEN, MIDIEventRecvDialog::OnListenClick)
	EVT_BUTTON(ID_EVENT_NEW, MIDIEventRecvDialog::OnNewClick)
	EVT_BUTTON(ID_EVENT_DELETE, MIDIEventRecvDialog::OnDeleteClick)
	EVT_CHOICE(ID_EVENT_NO, MIDIEventRecvDialog::OnEventChange)
	EVT_CHOICE(ID_EVENT, MIDIEventRecvDialog::OnTypeChange)
END_EVENT_TABLE()


MIDIEventRecvDialog::MIDIEventRecvDialog (wxWindow* parent, GOrgueMidiReceiver* event, GOrgueSettings& settings):
	wxPanel(parent, wxID_ANY),
	m_Settings(settings),
	m_original(event),
	m_midi(*event),
	m_listener()
{

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* sizer = new wxFlexGridSizer(7, 2, 5, 5);
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

	if (m_midi.GetType() == MIDI_RECV_MANUAL)
		sizer->Add(new wxStaticText(this, wxID_ANY, _("L&owest velocity:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	else
		sizer->Add(new wxStaticText(this, wxID_ANY, _("L&ower limit:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowValue = new wxSpinCtrl(this, ID_LOW_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowValue, 0);
	if (m_midi.GetType() == MIDI_RECV_MANUAL)
		box->Add(new wxStaticText(this, wxID_ANY, _("H&ighest velocity:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	else
		box->Add(new wxStaticText(this, wxID_ANY, _("&Upper limit:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_HighValue = new wxSpinCtrl(this, ID_HIGH_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighValue, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);
	SetSizer(topSizer);

	m_device->Append(_("Any device"));

	std::vector<wxString> device_names = m_Settings.GetMidiInDeviceList();
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

	if (m_midi.GetType() != MIDI_RECV_MANUAL)
	{
		m_eventtype->Append(_("RPN"), (void*)MIDI_M_RPN);
		m_eventtype->Append(_("NRPN"), (void*)MIDI_M_NRPN);
	}
	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
	{
		m_eventtype->Append(_("RPN Toogle"), (void*)MIDI_M_RPN_ON);
		m_eventtype->Append(_("RPN Toogle"), (void*)MIDI_M_RPN_OFF);
		m_eventtype->Append(_("NRPN Toogle"), (void*)MIDI_M_NRPN_ON);
		m_eventtype->Append(_("NRPN Toogle"), (void*)MIDI_M_NRPN_OFF);
	}

	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
		m_eventtype->Append(_("Sys Ex Johannus"), (void*)MIDI_M_SYSEX_JOHANNUS);

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
	if (m_original->GetOrganfile())
		m_original->GetOrganfile()->AddMidiListener(&m_listener);
}

MIDIEventRecvDialog::~MIDIEventRecvDialog()
{
	m_listener.SetCallback(NULL);
}

void MIDIEventRecvDialog::RegisterMIDIListener(GOrgueMidi* midi)
{
	if (midi)
		m_listener.Register(midi);
}

void MIDIEventRecvDialog::DoApply()
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

	m_original->Assign(m_midi);
}

void MIDIEventRecvDialog::OnTypeChange(wxCommandEvent& event)
{
	midi_match_message_type type = (midi_match_message_type)(intptr_t)m_eventtype->GetClientData(m_eventtype->GetSelection());
	if (m_original->HasLowerLimit(type) || m_midi.GetType() == MIDI_RECV_MANUAL || m_midi.GetType() == MIDI_RECV_ENCLOSURE)
		m_LowValue->Enable();
	else
		m_LowValue->Disable();
	if (m_original->HasUpperLimit(type) || m_midi.GetType() == MIDI_RECV_MANUAL || m_midi.GetType() == MIDI_RECV_ENCLOSURE)
		m_HighValue->Enable();
	else
               m_HighValue->Disable();
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
	m_LowValue->SetValue(e.low_value);
	m_HighValue->SetValue(e.high_value);
	wxCommandEvent event;
	OnTypeChange(event);
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
	e.low_value = m_LowValue->GetValue();
	e.high_value = m_HighValue->GetValue();
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
		m_listener.SetCallback(this);
	}
	else
	{
		m_listener.SetCallback(NULL);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void MIDIEventRecvDialog::OnMidiEvent(const GOrgueMidiEvent& event)
{
	if (event.GetMidiType() == MIDI_NONE)
		return;

	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	switch(event.GetMidiType())
	{
	case MIDI_NOTE:
		e.type = MIDI_M_NOTE;
		break;
	case MIDI_CTRL_CHANGE:
		e.type = MIDI_M_CTRL_CHANGE;
		break;
	case MIDI_PGM_CHANGE:
		e.type = MIDI_M_PGM_CHANGE;
		break;
	case MIDI_RPN:
		e.type = MIDI_M_RPN;
		break;
	case MIDI_NRPN:
		e.type = MIDI_M_NRPN;
		break;
	case MIDI_SYSEX_JOHANNUS:
		e.type = MIDI_M_SYSEX_JOHANNUS;
		break;

	default:
		return;
	}
	e.device = event.GetDevice();
	e.channel = event.GetChannel();
	if (m_midi.GetType() != MIDI_RECV_MANUAL)
		e.key = event.GetKey();
	e.low_key = 0;
	e.high_key = 127;
	e.low_value = m_midi.GetType() == MIDI_RECV_ENCLOSURE ? 0 : 1;
	e.high_value = 127;

	LoadEvent();

	m_listen->SetValue(false);
	m_listener.SetCallback(NULL);
	this->SetCursor(wxCursor(wxCURSOR_ARROW));
}
