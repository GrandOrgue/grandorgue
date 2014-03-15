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

#include "MIDIEventRecvDialog.h"

#include "GOrgueMidiEvent.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

BEGIN_EVENT_TABLE(MIDIEventRecvDialog, wxPanel)
	EVT_TOGGLEBUTTON(ID_LISTEN_SIMPLE, MIDIEventRecvDialog::OnListenSimpleClick)
	EVT_TOGGLEBUTTON(ID_LISTEN_ADVANCED, MIDIEventRecvDialog::OnListenAdvancedClick)
	EVT_TIMER(ID_TIMER, MIDIEventRecvDialog::OnTimer)
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
	m_listener(),
	m_ListenState(0),
	m_Timer(this, ID_TIMER)
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

	m_DataLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	box->Add(m_DataLabel, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_data = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 127);
	box->Add(m_data, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Lowest key:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowKey = new wxSpinCtrl(this, ID_LOW_KEY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowKey, 0);
	box->Add(new wxStaticText(this, wxID_ANY, _("&Highest key:")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	m_HighKey = new wxSpinCtrl(this, ID_HIGH_KEY, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighKey, 0);

	m_LowValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	sizer->Add(m_LowValueLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_LowValue = new wxSpinCtrl(this, ID_LOW_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_LowValue, 0);
	m_HighValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
	box->Add(m_HighValueLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	m_HighValue = new wxSpinCtrl(this, ID_HIGH_VALUE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 127);
	box->Add(m_HighValue, 0);

	sizer->Add(new wxStaticText(this, wxID_ANY, _("&Debounce time(ms):")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 15);
	box = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(box);
	m_Debounce = new wxSpinCtrl(this, ID_DEBOUNCE, wxEmptyString, wxDefaultPosition, wxSize(68, wxDefaultCoord), wxSP_ARROW_KEYS, 0, 3000);
	box->Add(m_Debounce, 0);

	m_ListenSimple = new wxToggleButton(this, ID_LISTEN_SIMPLE, _("&Listen for Event"));
	sizer->Add(m_ListenSimple, 0, wxTOP, 5);
	m_ListenAdvanced = new wxToggleButton(this, ID_LISTEN_ADVANCED, _("&Detect complex MIDI setup"));
	sizer->Add(m_ListenAdvanced, 0, wxTOP, 5);

	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_ListenInstructions = new wxStaticText(this, wxID_ANY, wxT(""));
	sizer->Add(m_ListenInstructions, 0, wxTOP, 5);

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
		m_eventtype->Append(_("9x Note without map"), (void*)MIDI_M_NOTE_NORMAL);
	}

	if (m_midi.GetType() != MIDI_RECV_MANUAL)
	{
		m_eventtype->Append(_("RPN"), (void*)MIDI_M_RPN);
		m_eventtype->Append(_("NRPN"), (void*)MIDI_M_NRPN);
		m_eventtype->Append(_("Cx Program Change Range"), (void*)MIDI_M_PGM_RANGE);
	}
	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
	{
		m_eventtype->Append(_("RPN Toogle"), (void*)MIDI_M_RPN_ON);
		m_eventtype->Append(_("RPN Toogle"), (void*)MIDI_M_RPN_OFF);
		m_eventtype->Append(_("NRPN Toogle"), (void*)MIDI_M_NRPN_ON);
		m_eventtype->Append(_("NRPN Toogle"), (void*)MIDI_M_NRPN_OFF);
		m_eventtype->Append(_("RPN Range"), (void*)MIDI_M_RPN_RANGE);
		m_eventtype->Append(_("NRPN Range"), (void*)MIDI_M_NRPN_RANGE);
	}

	if (m_midi.GetType() != MIDI_RECV_MANUAL && m_midi.GetType() != MIDI_RECV_ENCLOSURE)
	{
		m_eventtype->Append(_("Ctrl Change Bitfield"), (void*)MIDI_M_CTRL_BIT);
		m_eventtype->Append(_("Bx Ctrl Change Fixed Value"), (void*)MIDI_M_CTRL_CHANGE_FIXED);
		m_eventtype->Append(_("Bx Ctrl Change Fixed On Value Toogle"), (void*)MIDI_M_CTRL_CHANGE_FIXED_ON);
		m_eventtype->Append(_("Bx Ctrl Change Fixed Off Value Toogle"), (void*)MIDI_M_CTRL_CHANGE_FIXED_OFF);
		m_eventtype->Append(_("Sys Ex Johannus"), (void*)MIDI_M_SYSEX_JOHANNUS);
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
	StopListen();
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
	if (m_original->HasChannel(type))
		m_channel->Enable();
	else
		m_channel->Disable();
	if (m_original->HasDebounce(type))
		m_Debounce->Enable();
	else
		m_Debounce->Disable();
	if (m_original->HasLowKey(type))
		m_LowKey->Enable();
	else
		m_LowKey->Disable();
	if (m_original->HasHighKey(type))
		m_HighKey->Enable();
	else
               m_HighKey->Disable();
	if (m_original->HasLowerLimit(type))
	{
		m_LowValue->Enable();
		m_LowValue->SetRange(0, m_original->LowerValueLimit(type));
	}
	else
		m_LowValue->Disable();
	if (m_original->HasUpperLimit(type))
	{
		m_HighValue->Enable();
		m_HighValue->SetRange(0, m_original->UpperValueLimit(type));
	}
	else
               m_HighValue->Disable();
	if (m_original->HasKey(type))
	{
		m_data->Enable();
		if (m_midi.GetType() == MIDI_RECV_MANUAL)
			m_data->SetRange(-35, 35);
		else
			m_data->SetRange(0, m_original->KeyLimit(type));
	}
	else
		m_data->Disable();

	if (m_midi.GetType() == MIDI_RECV_MANUAL)
	{
		m_LowValueLabel->SetLabel(_("L&owest velocity:"));
		m_HighValueLabel->SetLabel(_("H&ighest velocity:"));
		m_DataLabel->SetLabel(_("&Transpose:"));
	}
	else
	{
		if (type == MIDI_M_CTRL_BIT)
			m_LowValueLabel->SetLabel(_("&Bit number:"));
		else if (type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_CTRL_CHANGE_FIXED_OFF)
			m_LowValueLabel->SetLabel(_("&Off value:"));
		else if (type == MIDI_M_PGM_RANGE)
			m_LowValueLabel->SetLabel(_("&Lower PGM number:"));
		else if (type == MIDI_M_RPN_RANGE)
			m_LowValueLabel->SetLabel(_("&Off RPN number:"));
		else if (type == MIDI_M_NRPN_RANGE)
			m_LowValueLabel->SetLabel(_("&Off NRPN number:"));
		else
			m_LowValueLabel->SetLabel(_("L&ower limit:"));

		if (type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_CTRL_CHANGE_FIXED_ON)
			m_HighValueLabel->SetLabel(_("&On value:"));
		else if (type == MIDI_M_PGM_RANGE)
			m_HighValueLabel->SetLabel(_("&Upper PGM number:"));
		else if (type == MIDI_M_RPN_RANGE)
			m_HighValueLabel->SetLabel(_("&On RPN number:"));
		else if (type == MIDI_M_NRPN_RANGE)
			m_HighValueLabel->SetLabel(_("&On NRPN number:"));
		else
			m_HighValueLabel->SetLabel(_("&Upper limit:"));

		switch(type)
		{
		case MIDI_M_CTRL_CHANGE:
		case MIDI_M_CTRL_BIT:
		case MIDI_M_CTRL_CHANGE_ON:
		case MIDI_M_CTRL_CHANGE_OFF:
		case MIDI_M_CTRL_CHANGE_FIXED:
		case MIDI_M_CTRL_CHANGE_FIXED_ON:
		case MIDI_M_CTRL_CHANGE_FIXED_OFF:
			m_DataLabel->SetLabel(_("&Controller-No:"));
			break;

		case MIDI_M_RPN:
		case MIDI_M_NRPN:
		case MIDI_M_RPN_ON:
		case MIDI_M_RPN_OFF:
		case MIDI_M_NRPN_ON:
		case MIDI_M_NRPN_OFF:
			m_DataLabel->SetLabel(_("&Parameter-No:"));
			break;

		case MIDI_M_RPN_RANGE:
		case MIDI_M_NRPN_RANGE:
			m_DataLabel->SetLabel(_("&Value:"));
			break;

		default:
			m_DataLabel->SetLabel(_("&Data:"));
		}
	}
	Layout();
}

void MIDIEventRecvDialog::LoadEvent()
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

	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);

	m_eventtype->SetSelection(0);
	for(unsigned i = 0; i < m_eventtype->GetCount(); i++)
		if ((void*)e.type == m_eventtype->GetClientData(i))
			m_eventtype->SetSelection(i);

	wxCommandEvent event;
	OnTypeChange(event);

	m_device->SetSelection(0);
	for(unsigned i = 1; i < m_device->GetCount(); i++)
		if (m_Settings.GetMidiMap().GetDeviceByID(e.device) == m_device->GetString(i))
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
	m_Debounce->SetValue(e.debounce_time);
}

void MIDIEventRecvDialog::StoreEvent()
{
	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	if (m_device->GetSelection() == 0)
		e.device = 0;
	else
		e.device = m_Settings.GetMidiMap().GetDeviceByString(m_device->GetStringSelection());

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
	e.debounce_time = m_Debounce->GetValue();
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

void MIDIEventRecvDialog::OnListenSimpleClick(wxCommandEvent& event)
{
	if (m_ListenSimple->GetValue())
		StartListen(false);
	else
		StopListen();
}

void MIDIEventRecvDialog::OnListenAdvancedClick(wxCommandEvent& event)
{
	if (m_ListenAdvanced->GetValue())
		StartListen(true);
	else
		StopListen();
}

void MIDIEventRecvDialog::OnTimer(wxTimerEvent& event)
{
	if (m_ListenState == 2)
	{
		wxString label;
		switch(m_midi.GetType())
		{
		case MIDI_RECV_MANUAL:
			label = _("Please press the highest key with minimal velocity");
			break;

		case MIDI_RECV_ENCLOSURE:
			label = _("Please fully close the enclosure");
			break;
			
		default:
			label = _("Please toggle it again (to off state, if possible)");
		}

		m_ListenInstructions->SetLabel(label);
		m_ListenState = 3;
	}
	else if (m_ListenState == 3)
		DetectEvent();
}

void MIDIEventRecvDialog::OnMidiEvent(const GOrgueMidiEvent& event)
{
	switch(event.GetMidiType())
	{
	case MIDI_NOTE:
	case MIDI_CTRL_CHANGE:
	case MIDI_PGM_CHANGE:
	case MIDI_RPN:
	case MIDI_NRPN:
	case MIDI_SYSEX_JOHANNUS:
		break;

	default:
		return;
	}

	if (m_ListenState == 1)
	{
		m_OnList.push_back(event);
		DetectEvent();
	}
	else if (m_ListenState == 2)
	{
		m_Timer.Stop();
		m_Timer.Start(1000, true);
		m_OnList.push_back(event);
	}
	else if (m_ListenState == 3)
	{
		m_OffList.push_back(event);
		m_Timer.Stop();
		m_Timer.Start(1000, true);
	}
}

void MIDIEventRecvDialog::StartListen(bool type)
{
	this->SetCursor(wxCursor(wxCURSOR_WAIT));
	m_listener.SetCallback(this);
	if (!type)
		m_ListenAdvanced->Disable();
	if (type)
		m_ListenSimple->Disable();
	m_OnList.clear();
	m_OffList.clear();
	if (!type)
	{
		m_ListenInstructions->SetLabel(_("Please press the MIDI element"));
		m_ListenState = 1;
	}
	else
	{
		m_ListenState = 2;
		wxString label;
		switch(m_midi.GetType())
		{
		case MIDI_RECV_MANUAL:
			label = _("Please press the lowest key with minimal velocity");
			break;

		case MIDI_RECV_ENCLOSURE:
			label = _("Please fully open the enclosure");
			break;
			
		default:
			label = _("Please toggle it (to on state, if possible)");
		}

		m_ListenInstructions->SetLabel(label);
	}
}

void MIDIEventRecvDialog::StopListen()
{
	m_listener.SetCallback(NULL);
	m_Timer.Stop();
	this->SetCursor(wxCursor(wxCURSOR_ARROW));
	m_ListenAdvanced->Enable();
	m_ListenSimple->Enable();
	m_ListenSimple->SetValue(false);
	m_ListenAdvanced->SetValue(false);
	m_ListenInstructions->SetLabel(wxEmptyString);
	m_ListenState = 0;
	m_OnList.clear();
	m_OffList.clear();
}

bool MIDIEventRecvDialog::SimilarEvent(const GOrgueMidiEvent& e1, const GOrgueMidiEvent& e2)
{
	if (e1.GetDevice() != e2.GetDevice())
		return false;
	if (e1.GetMidiType() != e2.GetMidiType())
		return false;
	if (e1.GetChannel() != e2.GetChannel())
		return false;

	if (e1.GetMidiType() == MIDI_PGM_CHANGE)
		return true;
	if (e1.GetKey() == e2.GetKey())
	{
		if (e1.GetMidiType() != MIDI_NOTE)
			return true;
	}
	return false;
}

void MIDIEventRecvDialog::DetectEvent()
{
	if (m_ListenState == 3)
	{
		for(unsigned i = 0; i < m_OnList.size(); i++)
		{
			if (i + 1 < m_OnList.size())
			{
				if (SimilarEvent(m_OnList[i], m_OnList[i + 1]))
						continue;
			}
			GOrgueMidiEvent on = m_OnList[i];
			for(unsigned j = 0; j < m_OffList.size(); j++)
			{
				if (j + 1 < m_OffList.size())
				{
					if (SimilarEvent(m_OffList[j], m_OffList[j + 1]))
					{
						if (m_OffList[j].GetMidiType() != MIDI_NOTE)
						continue;
					}
				}
				GOrgueMidiEvent off = m_OffList[j];
				if (on.GetDevice() != off.GetDevice())
					continue;
				if (on.GetChannel() != off.GetChannel())
					continue;
				if (on.GetMidiType() != off.GetMidiType())
					continue;
				if (m_midi.GetType() == MIDI_RECV_MANUAL)
				{
					if (on.GetMidiType() != MIDI_NOTE)
						continue;
					MIDI_MATCH_EVENT& e = m_midi.GetEvent(m_current);
					e.type = MIDI_M_NOTE;
					e.device = on.GetDevice();
					e.channel = on.GetChannel();
					e.low_key = on.GetKey();
					e.high_key = off.GetKey();
					e.low_value = std::min(on.GetValue(), off.GetValue());
					e.high_value = 127;
					e.debounce_time = 0;
					if ((on.GetKey() % 12) == 4)
					{
						e.type = MIDI_M_NOTE_SHORT_OCTAVE;
						e.low_key -= 4;
					}
					LoadEvent();
					StopListen();
					return;
				}
				bool is_range = false;
				if (on.GetValue() == off.GetValue() && 
				    on.GetKey() != off.GetKey() &&
				    (on.GetMidiType() == MIDI_RPN || on.GetMidiType() == MIDI_NRPN))
				{
					if (m_midi.GetType() == MIDI_RECV_ENCLOSURE)
						is_range = false;
					else
						is_range = true;
				}
				else if (on.GetMidiType() == MIDI_PGM_CHANGE)
				{
					is_range = true;
				}
				if (on.GetKey() != off.GetKey() && !is_range)
					continue;
				if (m_midi.GetType() == MIDI_RECV_ENCLOSURE)
				{
					MIDI_MATCH_EVENT& e = m_midi.GetEvent(m_current);
					unsigned low = off.GetValue();
					unsigned high = on.GetValue();
					int key = on.GetKey();
					switch(on.GetMidiType())
					{
					case MIDI_CTRL_CHANGE:
						e.type = MIDI_M_CTRL_CHANGE;
						break;
					case MIDI_RPN:
						e.type = MIDI_M_RPN;
						break;
					case MIDI_NRPN:
						e.type = MIDI_M_NRPN;
						break;
					case MIDI_PGM_CHANGE:
						e.type = MIDI_M_PGM_RANGE;
						low = off.GetKey();
						high = on.GetKey();
						key = -1;
						break;
					default:
						continue;
					}
					e.device = on.GetDevice();
					e.channel = on.GetChannel();
					e.key = key;
					e.low_key = 0;
					e.high_key = 0;
					e.high_value = high;
					e.low_value = low;
					LoadEvent();
					StopListen();
					return;
				}
				MIDI_MATCH_EVENT& e = m_midi.GetEvent(m_current);
				unsigned low = 0;
				unsigned high = 1;
				int key = on.GetKey();
				switch(on.GetMidiType())
				{
				case MIDI_NOTE:
					e.type = MIDI_M_NOTE;
					if (on.GetValue() > 0 && off.GetValue() > 0)
						e.type = MIDI_M_NOTE_ON;
					if (on.GetValue() == 0 && off.GetValue() == 0)
						e.type = MIDI_M_NOTE_OFF;
					break;
				case MIDI_CTRL_CHANGE:
					e.type = MIDI_M_CTRL_CHANGE;
					if (on.GetValue() == off.GetValue())
						e.type = on.GetValue() > 0 ? MIDI_M_CTRL_CHANGE_ON : MIDI_M_CTRL_CHANGE_OFF;
					for(unsigned k = 0; k < 7; k++)
						if (on.GetValue() == off.GetValue() + (1 << k))
						{
							e.type = MIDI_M_CTRL_BIT;
							low = k;
						}
					if (off.GetValue() != 0 && (on.GetValue() & 64) && (on.GetValue() & 63) == off.GetValue())
					{
						if (e.type != MIDI_M_CTRL_BIT || on.GetKey() >= 60)
						{
							high = on.GetValue();
							low = off.GetValue();
							e.type = MIDI_M_CTRL_CHANGE_FIXED;
						}
					}
					break;
				case MIDI_PGM_CHANGE:
					if (on.GetKey() == off.GetKey())
						e.type = MIDI_M_PGM_CHANGE;
					else
					{
						e.type = MIDI_M_PGM_RANGE;
						low = off.GetKey();
						high = on.GetKey();
						key = -1;
					}
					break;
				case MIDI_RPN:
					if (is_range)
					{
						e.type = MIDI_M_RPN_RANGE;
						key = on.GetValue();
						low = off.GetKey();
						high = on.GetKey();
						break;
					}
					e.type = MIDI_M_RPN;
					if (on.GetValue() == off.GetValue())
						e.type = on.GetValue() > 0 ? MIDI_M_RPN_ON : MIDI_M_RPN_OFF;
					break;
				case MIDI_NRPN:
					if (is_range)
					{
						e.type = MIDI_M_RPN_RANGE;
						key = on.GetValue();
						low = off.GetKey();
						high = on.GetKey();
						break;
					}
					e.type = MIDI_M_NRPN;
					if (on.GetValue() == off.GetValue())
						e.type = on.GetValue() > 0 ? MIDI_M_NRPN_ON : MIDI_M_NRPN_OFF;
					break;
				case MIDI_SYSEX_JOHANNUS:
					e.type = MIDI_M_SYSEX_JOHANNUS;
					break;

				default:
					continue;
				}
				e.device = on.GetDevice();
				e.channel = on.GetChannel();
				e.key = key;
				e.low_key = 0;
				e.high_key = 0;
				e.low_value = low;
				e.high_value = high;
				e.debounce_time = 0;
				LoadEvent();
				StopListen();
				return;
			}
		}
	}

	MIDI_MATCH_EVENT& e = m_midi.GetEvent(m_current);
	GOrgueMidiEvent& event = m_OnList[0];
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
		e.type = MIDI_M_NONE;
	}
	e.device = event.GetDevice();
	e.channel = event.GetChannel();
	if (m_midi.GetType() != MIDI_RECV_MANUAL)
		e.key = event.GetKey();
	e.low_key = 0;
	e.high_key = 127;
	e.low_value = m_midi.GetType() == MIDI_RECV_MANUAL ? 1 : 0;
	e.high_value = (m_midi.GetType() == MIDI_RECV_MANUAL || m_midi.GetType() == MIDI_RECV_ENCLOSURE) ? 127 : 1;
	e.debounce_time = 0;
	
	LoadEvent();
	StopListen();
}
