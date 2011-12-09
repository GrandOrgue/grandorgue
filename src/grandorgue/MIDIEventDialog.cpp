/*
 * GrandOrgue - free pipe organ simulator
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
#include "MIDIEventDialog.h"
#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"

extern GOrgueSound* g_sound;

IMPLEMENT_CLASS(MIDIEventDialog, wxDialog)

BEGIN_EVENT_TABLE(MIDIEventDialog, wxDialog)
	EVT_TOGGLEBUTTON(ID_LISTEN, MIDIEventDialog::OnListenClick)
	EVT_BUTTON(ID_EVENT_NEW, MIDIEventDialog::OnNewClick)
	EVT_BUTTON(ID_EVENT_DELETE, MIDIEventDialog::OnDeleteClick)
	EVT_CHOICE(ID_EVENT_NO, MIDIEventDialog::OnEventChange)
	EVT_MIDI(MIDIEventDialog::OnMidiEvent)
END_EVENT_TABLE()


MIDIEventDialog::MIDIEventDialog (wxWindow* parent, wxString title, const GOrgueMidiReceiver& event):
	wxDialog(parent, wxID_ANY, title),
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
	m_data = new wxSpinCtrl(this, ID_CHANNEL, wxEmptyString, wxDefaultPosition, wxSize(48, wxDefaultCoord), wxSP_ARROW_KEYS, -11, 127);
	box->Add(m_data, 0);

	if (m_midi.GetType() == MIDI_RECV_MANUAL)
		m_data->SetRange(-35, 35);
	else
		m_data->SetRange(0, 0x200000);


	sizer->Add(new wxStaticText(this, wxID_ANY, wxEmptyString));
	m_listen = new wxToggleButton(this, ID_LISTEN, _("&Listen for Event"));
	sizer->Add(m_listen, 0, wxTOP, 5);

	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 5);
	topSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->AddSpacer(5);
	SetSizer(topSizer);

	m_device->Append(_("Any device"));

	std::vector<wxString> device_names = m_midi.GetSettings().GetMidiDeviceList();
	for(std::vector<wxString>::iterator it = device_names.begin(); it != device_names.end(); it++)
		m_device->Append(*it);

	m_channel->Append(_("Any channel"), (void*)-1);
	for(unsigned int i = 1 ; i < 16; i++)
	{
		wxString str;
		str.Printf(wxT("%d"),  i);
		m_channel->Append(str, (void*)i);
	}

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

	m_current = 0;
	if (!m_midi.GetEventCount())
		m_midi.AddNewEvent();

	LoadEvent();

	topSizer->Fit(this);
}

MIDIEventDialog::~MIDIEventDialog()
{
	g_sound->GetMidi().SetListener(NULL);
}

const GOrgueMidiReceiver& MIDIEventDialog::GetResult()
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

void MIDIEventDialog::LoadEvent()
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
	for(unsigned i = 0; i < m_channel->GetCount(); i++)
		if ((void*)e.channel == m_channel->GetClientData(i))
			m_channel->SetSelection(i);

	m_data->SetValue(e.key);
}

void MIDIEventDialog::StoreEvent()
{
	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	if (m_device->GetSelection() == 0)
		e.device = wxT("");
	else
		e.device = m_device->GetStringSelection();

	e.type = (midi_match_message_type)(intptr_t) m_eventtype->GetClientData(m_eventtype->GetSelection());
	e.channel = (int)(intptr_t) m_channel->GetClientData(m_channel->GetSelection());

	e.key = m_data->GetValue();
}

void MIDIEventDialog::OnNewClick(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_midi.AddNewEvent();
	LoadEvent();
}

void MIDIEventDialog::OnDeleteClick(wxCommandEvent& event)
{
	m_midi.DeleteEvent(m_current);
	m_current = 0;
	LoadEvent();
}

void MIDIEventDialog::OnEventChange(wxCommandEvent& event)
{
	StoreEvent();
	m_current = m_eventno->GetSelection();
	LoadEvent();
}

void MIDIEventDialog::OnListenClick(wxCommandEvent& event)
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

void MIDIEventDialog::OnMidiEvent(GOrgueMidiEvent& event)
{
	if (event.GetMidiType() == MIDI_NONE)
		return;

	MIDI_MATCH_EVENT& e=m_midi.GetEvent(m_current);
	e.type = (midi_match_message_type)event.GetMidiType();
	e.device = event.GetDevice();
	e.channel = event.GetChannel();
	if (m_midi.GetType() != MIDI_RECV_MANUAL)
		e.key = event.GetKey();

	LoadEvent();

	m_listen->SetValue(false);
	g_sound->GetMidi().SetListener(NULL);
	this->SetCursor(wxCursor(wxCURSOR_ARROW));
}
