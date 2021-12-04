/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "SettingsMidiDevices.h"

#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/choicdlg.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "midi/GOMidi.h"
#include "midi/ports/GOMidiPortFactory.h"
#include "settings/GOSettings.h"
#include "sound/GOSound.h"

BEGIN_EVENT_TABLE(SettingsMidiDevices, wxPanel)
	EVT_LISTBOX(ID_INDEVICES, SettingsMidiDevices::OnInDevicesClick)
	EVT_LISTBOX_DCLICK(ID_INDEVICES, SettingsMidiDevices::OnInOutDeviceClick)
	EVT_BUTTON(ID_INCHANNELSHIFT, SettingsMidiDevices::OnInChannelShiftClick)
	EVT_BUTTON(ID_INOUTDEVICE, SettingsMidiDevices::OnInOutDeviceClick)
END_EVENT_TABLE()

SettingsMidiDevices::SettingsMidiDevices(GOSound& sound, wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	GOSettingsPorts(this, GOMidiPortFactory::getInstance(), _("Midi &ports")),
	m_Sound(sound),
	m_Settings(sound.GetSettings())
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* box1 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* midiPropSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("&Midi properties"));

	m_AutoAddInput = new wxCheckBox();
	m_CheckOnStartup = new(wxCheckBox);

	midiPropSizer->Add(m_AutoAddInput = new wxCheckBox(this, ID_AUTO_ADD_MIDI, _("Auto add new devices")), 0, wxEXPAND | wxALL, 5);
	midiPropSizer->Add(m_CheckOnStartup = new wxCheckBox(this, ID_CHECK_ON_STARTUP, _("Check on startup")), 0, wxEXPAND | wxALL, 5);

	box1->Add(midiPropSizer, 0, wxALL | wxALIGN_TOP, 5);

	box1->Add(GetPortsBox(), 1, wxEXPAND | wxALL, 5);
	topSizer->Add(box1, 0.5, wxEXPAND | wxALL, 5);
	wxBoxSizer* item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &input devices"));
	m_InDevices = new wxCheckListBox(this, ID_INDEVICES, wxDefaultPosition, wxSize(100, 100));
	item3->Add(m_InDevices, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* item4 = new wxBoxSizer(wxHORIZONTAL);
	m_InProperties = new wxButton(this, ID_INCHANNELSHIFT, _("A&dvanced..."));
	m_InProperties->Disable();
	item4->Add(m_InProperties, 0, wxALL, 5);

	m_InOutDevice = new wxButton(this, ID_INOUTDEVICE, _("&MIDI-Output-Device..."));
	m_InOutDevice->Disable();
	item4->Add(m_InOutDevice, 0, wxALL, 5);
	item3->Add(item4, 0, wxALIGN_RIGHT | wxALL, 5);
	topSizer->Add(item3, 1, wxEXPAND | wxALL, 5);

	item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &output devices"));
	m_OutDevices = new wxCheckListBox(this, ID_OUTDEVICES, wxDefaultPosition, wxSize(100, 100));
	item3->Add(m_OutDevices, 1, wxEXPAND | wxALL, 5);

	item4 = new wxBoxSizer(wxHORIZONTAL);
	item3->Add(
	  new wxStaticText(this, wxID_ANY, _("Send MIDI Recorder Output Stream to:")),
	  0, wxEXPAND | wxALIGN_LEFT | wxALL, 5
	);
	m_RecorderDevice = new wxChoice(
	  this, ID_RECORDERDEVICE, wxDefaultPosition, wxSize(100, wxDefaultCoord)
	);
	item3->Add(m_RecorderDevice, 0, wxEXPAND | wxALL, 5);
	topSizer->Add(item3, 1, wxEXPAND | wxALL, 5);

	topSizer->AddSpacer(5);
	this->SetSizer(topSizer);
	topSizer->Fit(this);

	m_AutoAddInput->SetValue(m_Settings.IsToAutoAddMidi());
	m_CheckOnStartup->SetValue(m_Settings.IsToCheckMidiOnStart());

	const GOPortsConfig& portsConfig = m_Settings.GetMidiPortsConfig();

	FillPortsWith(portsConfig);
	RenewDevices(portsConfig);
}

void SettingsMidiDevices::RenewDevices(const GOPortsConfig& portsConfig)
{
  GOMidi& midi(m_Sound.GetMidi());
  midi.UpdateDevices(portsConfig);

  m_InDevices->Clear();
  // Fill m_InDevices
  for (const wxString& deviceName : midi.GetInDevices())
  {
    const int i = m_InDevices->Append(deviceName);

    if (m_Settings.GetMidiInState(deviceName))
      m_InDevices->Check(i);
    m_InDeviceData.push_back(m_Settings.GetMidiInDeviceChannelShift(deviceName));
    m_InOutDeviceData.push_back(m_Settings.GetMidiInOutDevice(deviceName));
  }

  // Fill m_OutDevices and m_RecorderDevice
  m_RecorderDevice->Clear();
  m_RecorderDevice->Append(_("No device"));
  m_RecorderDevice->Select(0);
  m_OutDevices->Clear();
  for (const wxString& deviceName : midi.GetOutDevices())
  {
    const int iOut = m_OutDevices->Append(deviceName);
    const int iRec = m_RecorderDevice->Append(deviceName);

    if (m_Settings.GetMidiOutState(deviceName) == 1)
      m_OutDevices->Check(iOut);
    if (m_Settings.MidiRecorderOutputDevice() == deviceName)
      m_RecorderDevice->SetSelection(iRec);
  }
}

void SettingsMidiDevices::OnPortChanged(
  const wxString &portName, const wxString apiName, bool oldEnabled, bool newEnabled
)
{
  RenewDevices(RenewPortsConfig());
}

void SettingsMidiDevices::OnInDevicesClick(wxCommandEvent& event)
{
	m_InProperties->Enable();
	m_InOutDevice->Enable();
}

void SettingsMidiDevices::OnInOutDeviceClick(wxCommandEvent& event)
{
	int index = m_InDevices->GetSelection();
	wxArrayString choices = m_RecorderDevice->GetStrings();
	choices[0] = _("No device");
	int selection = 0;
	for(unsigned i = 1; i < choices.GetCount(); i++)
		if (choices[i] == m_InOutDeviceData[index])
			selection = i;
	int result = wxGetSingleChoiceIndex(_("Select the corresponding MIDI output device for converting input events into output events"), _("MIDI output device"), choices, selection, this);
	if (result == -1)
		return;
	if (result)
		m_InOutDeviceData[index] = choices[result];
	else
		m_InOutDeviceData[index] = wxEmptyString;
}

void SettingsMidiDevices::OnInChannelShiftClick(wxCommandEvent& event)
{
	m_InProperties->Enable();
	m_InOutDevice->Enable();
	int index = m_InDevices->GetSelection();
	int result = ::wxGetNumberFromUser(_("A channel offset allows the use of two MIDI\ninterfaces with conflicting MIDI channels. For\nexample, applying a channel offset of 8 to\none of the MIDI interfaces would cause that\ninterface's channel 1 to appear as channel 9,\nchannel 2 to appear as channel 10, and so on."), _("Channel offset:"), 
					   m_InDevices->GetString(index), m_InDeviceData[index], 0, 15, this);

	if (result >= 0)
		m_InDeviceData[index] = result;
}

void SettingsMidiDevices::Save()
{
	m_Settings.IsToAutoAddMidi(m_AutoAddInput->IsChecked());
	m_Settings.IsToCheckMidiOnStart(m_CheckOnStartup->IsChecked());
	m_Settings.SetMidiPortsConfig(RenewPortsConfig());

	for (unsigned i = 0; i < m_InDevices->GetCount(); i++)
	{
		m_Settings.SetMidiInState(m_InDevices->GetString(i), m_InDevices->IsChecked(i));
		m_Settings.SetMidiInDeviceChannelShift(m_InDevices->GetString(i), m_InDeviceData[i]);
		m_Settings.SetMidiInOutDevice(m_InDevices->GetString(i), m_InOutDeviceData[i]);
	}

	for (unsigned i = 0; i < m_OutDevices->GetCount(); i++)
	{
		m_Settings.SetMidiOutState(m_OutDevices->GetString(i), m_OutDevices->IsChecked(i));
	}
	if (m_RecorderDevice->GetSelection() == 0)
		m_Settings.MidiRecorderOutputDevice(wxEmptyString);
	else
		m_Settings.MidiRecorderOutputDevice(m_RecorderDevice->GetString(m_RecorderDevice->GetSelection()));
}
