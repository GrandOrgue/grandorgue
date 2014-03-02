/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#include "MIDIEventSendDialog.h"

#include "GOrgueSettings.h"
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

BEGIN_EVENT_TABLE(MIDIEventSendDialog, wxPanel)
	EVT_BUTTON(ID_EVENT_NEW, MIDIEventSendDialog::OnNewClick)
	EVT_BUTTON(ID_EVENT_DELETE, MIDIEventSendDialog::OnDeleteClick)
	EVT_CHOICE(ID_EVENT_NO, MIDIEventSendDialog::OnEventChange)
	EVT_CHOICE(ID_EVENT, MIDIEventSendDialog::OnTypeChange)
END_EVENT_TABLE()

MIDIEventSendDialog::MIDIEventSendDialog (wxWindow* parent, GOrgueMidiSender* event, GOrgueSettings& settings):
	wxPanel(parent, wxID_ANY),
	m_Settings(settings),
	m_original(event),
	m_midi(*event)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* sizer = new wxFlexGridSizer(6, 2, 5, 5);
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

	m_channel = new wxChoice(this, ID_CHANNEL);
	sizer->Add(m_channel, 1, wxEXPAND);

	m_KeyLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	sizer->Add(m_KeyLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	m_key = new wxSpinCtrl(this, ID_KEY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	sizer->Add(m_key, 0);

	m_LowValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	sizer->Add(m_LowValueLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowValue = new wxSpinCtrl(this, ID_LOW_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowValue, 0);
	m_HighValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	box->Add(m_HighValueLabel, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_HighValue = new wxSpinCtrl(this, ID_HIGH_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighValue, 0);
	SetSizer(topSizer);

	m_device->Append(_("Any device"));

	std::vector<wxString> device_names = m_Settings.GetMidiOutDeviceList();
	for(std::vector<wxString>::iterator it = device_names.begin(); it != device_names.end(); it++)
		m_device->Append(*it);

	for(unsigned int i = 1 ; i <= 16; i++)
		m_channel->Append(wxString::Format(wxT("%d"), i));;

	m_eventtype->Append(_("(none)"), (void*)MIDI_S_NONE);
	if (m_midi.GetType() == MIDI_SEND_MANUAL || m_midi.GetType() == MIDI_SEND_BUTTON)
		m_eventtype->Append(_("9x Note"), (void*)MIDI_S_NOTE);
	if (m_midi.GetType() == MIDI_SEND_MANUAL)
		m_eventtype->Append(_("9x Note without Velocity"), (void*)MIDI_S_NOTE_NO_VELOCITY);
	if (m_midi.GetType() == MIDI_SEND_BUTTON)
	{
		m_eventtype->Append(_("9x Note On"), (void*)MIDI_S_NOTE_ON);
		m_eventtype->Append(_("9x Note Off"), (void*)MIDI_S_NOTE_OFF);
	}
	if (m_midi.GetType() == MIDI_SEND_ENCLOSURE || m_midi.GetType() == MIDI_SEND_BUTTON)
		m_eventtype->Append(_("Bx Controller"), (void*)MIDI_S_CTRL);
	if (m_midi.GetType() == MIDI_SEND_BUTTON)
	{
		m_eventtype->Append(_("Bx Controller On"), (void*)MIDI_S_CTRL_ON);
		m_eventtype->Append(_("Bx Controller Off"), (void*)MIDI_S_CTRL_OFF);
	}
	if (m_midi.GetType() == MIDI_SEND_BUTTON)
	{
		m_eventtype->Append(_("Cx Program Change On"), (void*)MIDI_S_PGM_ON);
		m_eventtype->Append(_("Cx Program Change Off"), (void*)MIDI_S_PGM_OFF);
	}
	if (m_midi.GetType() == MIDI_SEND_ENCLOSURE || m_midi.GetType() == MIDI_SEND_BUTTON)
	{
		m_eventtype->Append(_("RPN"), (void*)MIDI_S_RPN);
		m_eventtype->Append(_("NRPN"), (void*)MIDI_S_NRPN);
	}
	if (m_midi.GetType() == MIDI_SEND_BUTTON)
	{
		m_eventtype->Append(_("RPN On"), (void*)MIDI_S_RPN_ON);
		m_eventtype->Append(_("RPN Off"), (void*)MIDI_S_RPN_OFF);
		m_eventtype->Append(_("NRPN On"), (void*)MIDI_S_NRPN_ON);
		m_eventtype->Append(_("NRPN Off"), (void*)MIDI_S_NRPN_OFF);
	}
	if (m_midi.GetType() == MIDI_SEND_LABEL)
	{
		m_eventtype->Append(_("SYSEX Hauptwerk 32 Byte LCD"), (void*)MIDI_S_HW_LCD);
		m_eventtype->Append(_("SYSEX Hauptwerk 16 Byte String"), (void*)MIDI_S_HW_STRING);
	}

	m_key->SetRange(0, 0x200000);

	m_current = 0;
	if (!m_midi.GetEventCount())
		m_midi.AddNewEvent();

	LoadEvent();

	topSizer->Fit(this);
}

MIDIEventSendDialog::~MIDIEventSendDialog()
{
}

void MIDIEventSendDialog::DoApply()
{
	StoreEvent();

	bool empty_event;
	do
	{
		empty_event = false;
		for(unsigned i = 0; i < m_midi.GetEventCount(); i++)
			if (m_midi.GetEvent(i).type == MIDI_S_NONE)
			{
				m_midi.DeleteEvent(i);
				empty_event = true;
			}
	}
	while(empty_event);

	m_original->Assign(m_midi);
}

void MIDIEventSendDialog::OnTypeChange(wxCommandEvent& event)
{
	midi_send_message_type type = (midi_send_message_type)(intptr_t)m_eventtype->GetClientData(m_eventtype->GetSelection());
	if (m_original->HasChannel(type))
		m_channel->Enable();
	else
		m_channel->Disable();
	if (m_original->HasKey(type))
		m_key->Enable();
	else
		m_key->Disable();
	if (m_original->HasLowValue(type))
		m_LowValue->Enable();
	else
		m_LowValue->Disable();
	if (m_original->HasHighValue(type))
		m_HighValue->Enable();
	else
		m_HighValue->Disable();
	if (type == MIDI_S_HW_LCD)
		m_LowValueLabel->SetLabel(_("&Color:"));
	else
		m_LowValueLabel->SetLabel(_("&Off Value:"));
	m_HighValueLabel->SetLabel(_("&On value:"));
	switch(type)
	{
	case MIDI_S_CTRL_ON:
	case MIDI_S_CTRL_OFF:
	case MIDI_S_CTRL:
		m_KeyLabel->SetLabel(_("&Controller-No:"));
		break;

	case MIDI_S_RPN:
	case MIDI_S_NRPN:
	case MIDI_S_RPN_ON:
	case MIDI_S_RPN_OFF:
	case MIDI_S_NRPN_ON:
	case MIDI_S_NRPN_OFF:
		m_KeyLabel->SetLabel(_("&Parameter-No:"));
		break;

	case MIDI_S_HW_STRING:
	case MIDI_S_HW_LCD:
		m_KeyLabel->SetLabel(_("&ID:"));
		break;

	default:
		m_KeyLabel->SetLabel(_("&CTRL/PGM:"));
	}
	Layout();
}

void MIDIEventSendDialog::LoadEvent()
{
	m_eventno->Clear();
	for(unsigned i = 0; i < m_midi.GetEventCount(); i++)
		{
			wxString buffer;
			wxString device;
			if (m_midi.GetEvent(i).device == 0)
				device =  _("Any device");
			else
				device = m_Settings.GetMidiMap().GetDeviceByID(m_midi.GetEvent(i).device);
			buffer.Printf(_("%d (%s)"), i + 1, device.c_str());
			m_eventno->Append(buffer);
		}
	m_eventno->Select(m_current);
	if (m_midi.GetEventCount() > 1)
		m_delete->Enable();
	else
		m_delete->Disable();

	MIDI_SEND_EVENT& e=m_midi.GetEvent(m_current);

	m_eventtype->SetSelection(0);
	for(unsigned i = 0; i < m_eventtype->GetCount(); i++)
		if ((void*)e.type == m_eventtype->GetClientData(i))
			m_eventtype->SetSelection(i);

	m_device->SetSelection(0);
	for(unsigned i = 1; i < m_device->GetCount(); i++)
		if (m_Settings.GetMidiMap().GetDeviceByID(e.device) == m_device->GetString(i))
			m_device->SetSelection(i);

	m_channel->SetSelection(e.channel - 1);
	m_key->SetValue(e.key);
	m_LowValue->SetValue(e.low_value);
	m_HighValue->SetValue(e.high_value);

	wxCommandEvent event;
	OnTypeChange(event);
}

void MIDIEventSendDialog::StoreEvent()
{
	MIDI_SEND_EVENT& e=m_midi.GetEvent(m_current);
	if (m_device->GetSelection() == 0)
		e.device = 0;
	else
		e.device = m_Settings.GetMidiMap().GetDeviceByString(m_device->GetStringSelection());

	e.type = (midi_send_message_type)(intptr_t) m_eventtype->GetClientData(m_eventtype->GetSelection());
	e.channel = m_channel->GetSelection() + 1;
	e.key = m_key->GetValue();
	e.low_value = m_LowValue->GetValue();
	e.high_value = m_HighValue->GetValue();
}

void MIDIEventSendDialog::OnNewClick(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_midi.AddNewEvent();
	LoadEvent();
}

void MIDIEventSendDialog::OnDeleteClick(wxCommandEvent& event)
{
	m_midi.DeleteEvent(m_current);
	m_current = 0;
	LoadEvent();
}

void MIDIEventSendDialog::OnEventChange(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_eventno->GetSelection();
	LoadEvent();
}
