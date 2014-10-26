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

#include "SettingsMidiDevices.h"

#include "GOrgueMidi.h"
#include "GOrgueSettings.h"
#include "GOrgueSound.h"
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_EVENT_TABLE(SettingsMidiDevices, wxPanel)
	EVT_LISTBOX(ID_INDEVICES, SettingsMidiDevices::OnInDevicesClick)
	EVT_LISTBOX_DCLICK(ID_INDEVICES, SettingsMidiDevices::OnInDevicesDoubleClick)
	EVT_BUTTON(ID_INPROPERTIES, SettingsMidiDevices::OnInDevicesDoubleClick)
END_EVENT_TABLE()

SettingsMidiDevices::SettingsMidiDevices(GOrgueSound& sound, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Sound(sound)
{
	m_Sound.GetMidi().UpdateDevices();

	wxArrayString choices;
	std::vector<wxString> list = m_Sound.GetMidi().GetInDevices();
	std::vector<bool> state;
	for (unsigned i = 0; i < list.size(); i++)
	{
		choices.push_back(list[i]);
		m_InDeviceData.push_back(m_Sound.GetSettings().GetMidiInDeviceChannelShift(list[i]));
		state.push_back(m_Sound.GetSettings().GetMidiInState(list[i]));
	}

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &input devices"));
	m_InDevices = new wxCheckListBox(this, ID_INDEVICES, wxDefaultPosition, wxDefaultSize, choices);
	for (unsigned i = 0; i < state.size(); i++)
	{
		if (state[i])
			m_InDevices->Check(i);
	}
	item3->Add(m_InDevices, 1, wxEXPAND | wxALL, 5);
	m_InProperties = new wxButton(this, ID_INPROPERTIES, _("A&dvanced..."));
	m_InProperties->Disable();
	item3->Add(m_InProperties, 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->Add(item3, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);

	choices.clear();
	std::vector<bool> out_state;
	list = m_Sound.GetMidi().GetOutDevices();
	for (unsigned i = 0; i < list.size(); i++)
	{
		choices.push_back(list[i]);
		out_state.push_back(m_Sound.GetSettings().GetMidiOutState(list[i]) == 1);
	}

	item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &output devices"));
	m_OutDevices = new wxCheckListBox(this, ID_OUTDEVICES, wxDefaultPosition, wxDefaultSize, choices);
	for (unsigned i = 0; i < out_state.size(); i++)
		if (out_state[i])
			m_OutDevices->Check(i);

	item3->Add(m_OutDevices, 1, wxEXPAND | wxALL, 5);
	wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
	item3->Add(box);
	box->Add(new wxStaticText(this, wxID_ANY, _("Send MIDI Recorder Output Stream to ")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	m_RecorderDevice = new wxChoice(this, ID_RECORDERDEVICE);
	box->Add(m_RecorderDevice, 0);
	m_RecorderDevice->Append(_("No device"));
	m_RecorderDevice->Select(0);
	list = m_Sound.GetMidi().GetOutDevices();
	for (unsigned i = 0; i < list.size(); i++)
	{
		m_RecorderDevice->Append(list[i]);
		if (m_Sound.GetSettings().GetMidiRecorderOutputDevice() == list[i])
			m_RecorderDevice->SetSelection(m_RecorderDevice->GetCount() - 1);
	}

	topSizer->Add(item3, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsMidiDevices::OnInDevicesClick(wxCommandEvent& event)
{
	m_InProperties->Enable();
}

void SettingsMidiDevices::OnInDevicesDoubleClick(wxCommandEvent& event)
{
	m_InProperties->Enable();
	int index = m_InDevices->GetSelection();
	int result = ::wxGetNumberFromUser(_("A channel offset allows the use of two MIDI\ninterfaces with conflicting MIDI channels. For\nexample, applying a channel offset of 8 to\none of the MIDI interfaces would cause that\ninterface's channel 1 to appear as channel 9,\nchannel 2 to appear as channel 10, and so on."), _("Channel offset:"), 
					   m_InDevices->GetString(index), m_InDeviceData[index], 0, 15, this);

	if (result >= 0)
		m_InDeviceData[index] = result;
}

void SettingsMidiDevices::Save()
{
	for (unsigned i = 0; i < m_InDevices->GetCount(); i++)
	{
		m_Sound.GetSettings().SetMidiInState(m_InDevices->GetString(i), m_InDevices->IsChecked(i));
		m_Sound.GetSettings().SetMidiInDeviceChannelShift(m_InDevices->GetString(i), m_InDeviceData[i]);
	}

	for (unsigned i = 0; i < m_OutDevices->GetCount(); i++)
	{
		m_Sound.GetSettings().SetMidiOutState(m_OutDevices->GetString(i), m_OutDevices->IsChecked(i));
	}
	if (m_RecorderDevice->GetSelection() == 0)
		m_Sound.GetSettings().SetMidiRecorderOutputDevice(wxEmptyString);
	else
		m_Sound.GetSettings().SetMidiRecorderOutputDevice(m_RecorderDevice->GetString(m_RecorderDevice->GetSelection()));
}
