/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOSettingsMidiDevices.h"

#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/numdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"
#include "midi/GOMidi.h"
#include "midi/ports/GOMidiPortFactory.h"

BEGIN_EVENT_TABLE(SettingsMidiDevices, wxPanel)
EVT_LISTBOX(ID_INDEVICES, SettingsMidiDevices::OnInDevicesClick)
EVT_LISTBOX_DCLICK(ID_INDEVICES, SettingsMidiDevices::OnInOutDeviceClick)
EVT_BUTTON(ID_INCHANNELSHIFT, SettingsMidiDevices::OnInChannelShiftClick)
EVT_BUTTON(ID_INOUTDEVICE, SettingsMidiDevices::OnInOutDeviceClick)
EVT_LISTBOX(ID_OUTDEVICES, SettingsMidiDevices::OnOutDevicesClick)
END_EVENT_TABLE()

SettingsMidiDevices::SettingsMidiDevices(
  GOConfig &settings, GOMidi &midi, wxWindow *parent)
  : wxPanel(parent, wxID_ANY),
    GOSettingsPorts(this, GOMidiPortFactory::getInstance(), _("Midi &ports")),
    m_config(settings),
    m_Midi(midi),
    m_InDevices(m_Midi.GetInDevices(), m_config.m_MidiIn, this, ID_INDEVICES),
    m_OutDevices(
      m_Midi.GetOutDevices(), m_config.m_MidiOut, this, ID_OUTDEVICES) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *box1 = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer *midiPropSizer
    = new wxStaticBoxSizer(wxVERTICAL, this, _("&Midi properties"));

  m_AutoAddInput = new wxCheckBox();
  m_CheckOnStartup = new (wxCheckBox);

  midiPropSizer->Add(
    m_AutoAddInput
    = new wxCheckBox(this, ID_AUTO_ADD_MIDI, _("Auto add new devices")),
    0,
    wxEXPAND | wxALL,
    5);
  midiPropSizer->Add(
    m_CheckOnStartup
    = new wxCheckBox(this, ID_CHECK_ON_STARTUP, _("Check on startup")),
    0,
    wxEXPAND | wxALL,
    5);

  box1->Add(midiPropSizer, 0, wxALL | wxALIGN_TOP, 5);

  box1->Add(GetPortsBox(), 1, wxEXPAND | wxALL, 5);
  topSizer->Add(box1, 0.5, wxEXPAND | wxALL, 5);
  wxBoxSizer *item3
    = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &input devices"));
  item3->Add(
    m_InDevices.GetListbox(), 1, wxEXPAND | wxUP | wxRIGHT | wxLEFT, 5);

  wxBoxSizer *item4 = new wxBoxSizer(wxHORIZONTAL);
  m_InProperties = new wxButton(this, ID_INCHANNELSHIFT, _("A&dvanced..."));
  m_InProperties->Disable();
  item4->Add(m_InProperties, 0, wxALL, 5);

  m_InOutDevice
    = new wxButton(this, ID_INOUTDEVICE, _("&MIDI-Output-Device..."));
  m_InOutDevice->Disable();
  item4->Add(m_InOutDevice, 0, wxALL, 5);

  item4->Add(m_InDevices.GetMatchingButton(), 0, wxALL, 5);

  item3->Add(item4, 0, wxALIGN_RIGHT);
  topSizer->Add(item3, 1, wxEXPAND | wxDOWN | wxRIGHT | wxLEFT, 5);

  item3 = new wxStaticBoxSizer(wxVERTICAL, this, _("MIDI &output devices"));
  item3->Add(m_OutDevices.GetListbox(), 1, wxEXPAND | wxALL, 5);

  wxGridBagSizer *const bottomGb = new wxGridBagSizer(5, 5);

  bottomGb->AddGrowableCol(0, 1);
  bottomGb->Add(
    new wxStaticText(this, wxID_ANY, _("Send MIDI Recorder Output Stream to:")),
    wxGBPosition(0, 0),
    wxDefaultSpan,
    wxALIGN_LEFT | wxALIGN_BOTTOM | wxALL);
  m_RecorderDevice = new wxChoice(
    this, ID_RECORDERDEVICE, wxDefaultPosition, wxSize(100, wxDefaultCoord));
  bottomGb->Add(
    m_RecorderDevice, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL);
  bottomGb->Add(
    m_OutDevices.GetMatchingButton(),
    wxGBPosition(0, 1),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxDOWN,
    5);
  item3->Add(bottomGb, 0, wxEXPAND | wxDOWN | wxRIGHT | wxLEFT, 5);

  topSizer->Add(item3, 1, wxEXPAND | wxALL, 5);

  // topSizer->AddSpacer(5);
  this->SetSizer(topSizer);
  topSizer->Fit(this);

  const bool isToAutoAddInput = m_config.IsToAutoAddMidi();

  m_AutoAddInput->SetValue(isToAutoAddInput);
  m_CheckOnStartup->SetValue(m_config.IsToCheckMidiOnStart());

  const GOPortsConfig &portsConfig = m_config.GetMidiPortsConfig();

  FillPortsWith(portsConfig);
  RenewDevices(portsConfig, isToAutoAddInput);
}

void SettingsMidiDevices::RenewDevices(
  const GOPortsConfig &portsConfig, const bool isToAutoAddInput) {
  m_InProperties->Disable();
  m_InOutDevice->Disable();
  m_Midi.UpdateDevices(portsConfig);
  m_OutDevices.RefreshDevices(portsConfig, false);
  m_InDevices.RefreshDevices(portsConfig, isToAutoAddInput, &m_OutDevices);

  // Fill m_RecorderDevice
  m_RecorderDevice->Clear();
  m_RecorderDevice->Append(_("No device"));
  m_RecorderDevice->Select(0);
  for (unsigned l = m_OutDevices.GetDeviceCount(), i = 0; i < l; i++) {
    const GOMidiDeviceConfig &c = m_OutDevices.GetDeviceConf(i);

    m_RecorderDevice->Append(c.m_PhysicalName);
    if (m_config.MidiRecorderOutputDevice() == c.m_LogicalName)
      m_RecorderDevice->SetSelection(i + 1);
  }
}

void SettingsMidiDevices::OnPortChanged(
  const wxString &portName,
  const wxString apiName,
  bool oldEnabled,
  bool newEnabled) {
  RenewDevices(RenewPortsConfig(), m_AutoAddInput->IsChecked());
}

void SettingsMidiDevices::OnInDevicesClick(wxCommandEvent &event) {
  m_InProperties->Enable();
  m_InOutDevice->Enable();
  m_InDevices.OnSelected(event);
}

void SettingsMidiDevices::OnInOutDeviceClick(wxCommandEvent &event) {
  GOMidiDeviceConfig &devConf = m_InDevices.GetSelectedDeviceConf();
  wxArrayString choices = m_RecorderDevice->GetStrings();
  int selection = 0;

  choices[0] = _("No device");

  if (devConf.p_OutputDevice)
    for (unsigned i = 1; i < choices.GetCount(); i++)
      if (choices[i] == devConf.p_OutputDevice->m_PhysicalName)
        selection = i;

  int result = wxGetSingleChoiceIndex(
    _("Select the corresponding MIDI output device for converting input "
      "events into output events"),
    _("MIDI output device"),
    choices,
    selection,
    this);

  if (result >= 0)
    devConf.p_OutputDevice
      = result ? &m_OutDevices.GetDeviceConf(result - 1) : NULL;
}

void SettingsMidiDevices::OnInChannelShiftClick(wxCommandEvent &event) {
  GOMidiDeviceConfig &devConf = m_InDevices.GetSelectedDeviceConf();
  int result = ::wxGetNumberFromUser(
    _("A channel offset allows the use of two MIDI\n"
      "interfaces with conflicting MIDI channels. For\n"
      "example, applying a channel offset of 8 to\n"
      "one of the MIDI interfaces would cause that\n"
      "interface's channel 1 to appear as channel 9,\n"
      "channel 2 to appear as channel 10, and so on."),
    _("Channel offset:"),
    devConf.m_PhysicalName,
    devConf.m_ChannelShift,
    0,
    15,
    this);

  if (result >= 0)
    devConf.m_ChannelShift = result;
}

void SettingsMidiDevices::OnOutDevicesClick(wxCommandEvent &event) {
  m_OutDevices.OnSelected(event);
}

bool SettingsMidiDevices::TransferDataFromWindow() {
  m_config.IsToAutoAddMidi(m_AutoAddInput->IsChecked());
  m_config.IsToCheckMidiOnStart(m_CheckOnStartup->IsChecked());
  m_config.SetMidiPortsConfig(RenewPortsConfig());
  m_OutDevices.Save();
  m_InDevices.Save(&m_OutDevices);

  const int iRec = m_RecorderDevice->GetSelection();

  if (iRec <= 0)
    m_config.MidiRecorderOutputDevice(wxEmptyString);
  else
    m_config.MidiRecorderOutputDevice(
      m_OutDevices.GetDeviceConf(iRec - 1).m_LogicalName);
  return true;
}
