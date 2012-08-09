/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#include <wx/numdlg.h>

#include "SettingsMidiDevices.h"
#include "GOrgueMidi.h"
#include "GOrgueSound.h"
#include "GOrgueSettings.h"

BEGIN_EVENT_TABLE(SettingsMidiDevices, wxPanel)
	EVT_LISTBOX(ID_DEVICES, SettingsMidiDevices::OnDevicesClick)
	EVT_LISTBOX_DCLICK(ID_DEVICES, SettingsMidiDevices::OnDevicesDoubleClick)
	EVT_BUTTON(ID_PROPERTIES, SettingsMidiDevices::OnDevicesDoubleClick)
END_EVENT_TABLE()

SettingsMidiDevices::SettingsMidiDevices(GOrgueSound& sound, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	m_Sound(sound)
{
	m_Sound.GetMidi().UpdateDevices();

	wxArrayString choices;
	for (std::map<wxString, int>::iterator it2 = m_Sound.GetMidi().GetDevices().begin(); it2 != m_Sound.GetMidi().GetDevices().end(); it2++)
	{
		choices.push_back(it2->first);
		m_DeviceData.push_back(m_Sound.GetSettings().GetMidiInDeviceChannelShift(it2->first));
	}

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &input devices"));
	m_Devices = new wxCheckListBox(this, ID_DEVICES, wxDefaultPosition, wxDefaultSize, choices);
	for (unsigned i = 0; i < m_DeviceData.size(); i++)
	{
		if (m_DeviceData[i] < 0)
			m_DeviceData[i] = -m_DeviceData[i] - 1;
		else
			m_Devices->Check(i);
	}
	item3->Add(m_Devices, 1, wxEXPAND | wxALL, 5);
	m_Properties = new wxButton(this, ID_PROPERTIES, _("A&dvanced..."));
	m_Properties->Disable();
	item3->Add(m_Properties, 0, wxALIGN_RIGHT | wxALL, 5);

	topSizer->Add(item3, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 5);
	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);
}

void SettingsMidiDevices::OnDevicesClick(wxCommandEvent& event)
{
	m_Properties->Enable();
}

void SettingsMidiDevices::OnDevicesDoubleClick(wxCommandEvent& event)
{
	m_Properties->Enable();
	int index = m_Devices->GetSelection();
	int result = ::wxGetNumberFromUser(_("A channel offset allows the use of two MIDI\ninterfaces with conflicting MIDI channels. For\nexample, applying a channel offset of 8 to\none of the MIDI interfaces would cause that\ninterface's channel 1 to appear as channel 9,\nchannel 2 to appear as channel 10, and so on."), _("Channel offset:"), 
					   m_Devices->GetString(index), m_DeviceData[index], 0, 15, this);

	if (result >= 0)
		m_DeviceData[index] = result;
}

void SettingsMidiDevices::Save()
{
	for (unsigned i = 0; i < m_Devices->GetCount(); i++)
	{
		int j;

		j = m_DeviceData[i];
		if (!m_Devices->IsChecked(i))
			j = -j - 1;
		m_Sound.GetSettings().SetMidiInDeviceChannelShift(m_Devices->GetString(i), j);
	}
}
