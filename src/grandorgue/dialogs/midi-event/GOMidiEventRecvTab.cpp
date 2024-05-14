/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventRecvTab.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

#include "GOOrganController.h"
#include "GORodgers.h"
#include "config/GOConfig.h"
#include "midi/GOMidiEvent.h"

BEGIN_EVENT_TABLE(GOMidiEventRecvTab, wxPanel)
EVT_TOGGLEBUTTON(ID_LISTEN_SIMPLE, GOMidiEventRecvTab::OnListenSimpleClick)
EVT_TOGGLEBUTTON(ID_LISTEN_ADVANCED, GOMidiEventRecvTab::OnListenAdvancedClick)
EVT_TIMER(ID_TIMER, GOMidiEventRecvTab::OnTimer)
EVT_BUTTON(ID_EVENT_NEW, GOMidiEventRecvTab::OnNewClick)
EVT_BUTTON(ID_EVENT_DELETE, GOMidiEventRecvTab::OnDeleteClick)
EVT_CHOICE(ID_EVENT_NO, GOMidiEventRecvTab::OnEventChange)
EVT_CHOICE(ID_EVENT, GOMidiEventRecvTab::OnTypeChange)
END_EVENT_TABLE()

GOMidiEventRecvTab::GOMidiEventRecvTab(
  wxWindow *parent, GOMidiReceiverBase *event, GOConfig &config)
  : wxPanel(parent, wxID_ANY),
    m_MidiIn(config.m_MidiIn),
    m_MidiMap(config.GetMidiMap()),
    m_original(event),
    m_midi(*event),
    m_ReceiverType(event->GetType()),
    m_listener(),
    m_ListenState(0),
    m_Timer(this, ID_TIMER) {
  wxBoxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
  wxGridBagSizer *const grid = new wxGridBagSizer(5, 5);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("Event-&No")),
    wxGBPosition(0, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

  wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);

  m_eventno = new wxChoice(this, ID_EVENT_NO);
  box->Add(m_eventno, 1, wxRIGHT, 5);
  m_new = new wxButton(this, ID_EVENT_NEW, _("New"));
  box->Add(m_new, 0, wxLEFT | wxRIGHT, 5);
  m_delete = new wxButton(this, ID_EVENT_DELETE, _("Delete"));
  box->Add(m_delete, 0, wxLEFT | wxRIGHT, 5);
  grid->Add(box, wxGBPosition(0, 1), wxGBSpan(1, 4), wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Device:")),
    wxGBPosition(1, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

  m_device = new wxChoice(this, ID_EVENT);
  grid->Add(m_device, wxGBPosition(1, 1), wxGBSpan(1, 4), wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Event:")),
    wxGBPosition(2, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_eventtype = new GOChoice<GOMidiReceiverMessageType>(this, ID_EVENT);
  grid->Add(m_eventtype, wxGBPosition(2, 1), wxGBSpan(1, 4), wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Channel:")),
    wxGBPosition(3, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_channel = new wxChoice(this, ID_CHANNEL);
  grid->Add(m_channel, wxGBPosition(3, 1));

  m_DataLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_DataLabel,
    wxGBPosition(3, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_data = new wxSpinCtrl(
    this,
    ID_CHANNEL,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    -11,
    127);
  grid->Add(m_data, wxGBPosition(3, 3));

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Lowest key:")),
    wxGBPosition(4, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_LowKey = new wxSpinCtrl(
    this,
    ID_LOW_KEY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_LowKey, wxGBPosition(4, 1));
  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Highest key:")),
    wxGBPosition(4, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT,
    5);
  m_HighKey = new wxSpinCtrl(
    this,
    ID_HIGH_KEY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_HighKey, wxGBPosition(4, 3));

  m_LowValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_LowValueLabel,
    wxGBPosition(5, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_LowValue = new wxSpinCtrl(
    this,
    ID_LOW_VALUE,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_LowValue, wxGBPosition(5, 1));
  m_HighValueLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_HighValueLabel,
    wxGBPosition(5, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT,
    5);
  m_HighValue = new wxSpinCtrl(
    this,
    ID_HIGH_VALUE,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_HighValue, wxGBPosition(5, 3));

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Debounce time(ms):")),
    wxGBPosition(6, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_Debounce = new wxSpinCtrl(
    this,
    ID_DEBOUNCE,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    3000);
  grid->Add(m_Debounce, wxGBPosition(6, 1));

  grid->AddGrowableCol(4, 1);
  topSizer->Add(grid, 0, wxEXPAND | wxALL, 5);

  box = new wxBoxSizer(wxHORIZONTAL);
  m_ListenSimple
    = new wxToggleButton(this, ID_LISTEN_SIMPLE, _("&Listen for Event"));
  box->Add(m_ListenSimple, 0, wxALL, 5);
  m_ListenAdvanced = new wxToggleButton(
    this, ID_LISTEN_ADVANCED, _("&Detect complex MIDI setup"));
  box->Add(m_ListenAdvanced, 0, wxALL, 5);

  topSizer->Add(box, 0, wxEXPAND | wxALL, 5);

  m_ListenInstructions = new wxStaticText(this, wxID_ANY, wxT(""));
  topSizer->Add(m_ListenInstructions, 0, wxEXPAND | wxALL, 5);

  SetSizer(topSizer);

  const GOPortsConfig &portsConfig = config.GetMidiPortsConfig();

  m_device->Append(_("Any device"));
  for (GOMidiDeviceConfig *pDevConf : m_MidiIn)
    if (
      portsConfig.IsEnabled(pDevConf->m_PortName, pDevConf->m_ApiName)
      && pDevConf->m_IsEnabled)
      m_device->Append(pDevConf->m_LogicalName);

  m_channel->Append(_("Any channel"));
  for (unsigned int i = 1; i <= 16; i++)
    m_channel->Append(wxString::Format(wxT("%d"), i));
  ;

  m_eventtype->Append(_("(none)"), MIDI_M_NONE);
  if (m_ReceiverType != MIDI_RECV_ENCLOSURE)
    m_eventtype->Append(_("9x Note"), MIDI_M_NOTE);
  if (
    m_ReceiverType != MIDI_RECV_MANUAL
    && m_ReceiverType != MIDI_RECV_ENCLOSURE) {
    m_eventtype->Append(_("9x Note On Toggle"), MIDI_M_NOTE_ON);
    m_eventtype->Append(_("9x Note Off Toggle"), MIDI_M_NOTE_OFF);
    m_eventtype->Append(_("9x Note On/Off Toggle"), MIDI_M_NOTE_ON_OFF);
    m_eventtype->Append(_("9x Note Fixed On"), MIDI_M_NOTE_FIXED_ON);
    m_eventtype->Append(_("9x Note Fixed Off"), MIDI_M_NOTE_FIXED_OFF);
  }
  if (m_ReceiverType != MIDI_RECV_MANUAL)
    m_eventtype->Append(_("Bx Controller"), MIDI_M_CTRL_CHANGE);
  if (
    m_ReceiverType != MIDI_RECV_MANUAL
    && m_ReceiverType != MIDI_RECV_ENCLOSURE) {
    m_eventtype->Append(_("Bx Controller On Toggle"), MIDI_M_CTRL_CHANGE_ON);
    m_eventtype->Append(_("Bx Controller Off Toggle"), MIDI_M_CTRL_CHANGE_OFF);
    m_eventtype->Append(
      _("Bx Controller On/Off Toggle"), MIDI_M_CTRL_CHANGE_ON_OFF);
  }
  if (
    m_ReceiverType != MIDI_RECV_MANUAL && m_ReceiverType != MIDI_RECV_ENCLOSURE)
    m_eventtype->Append(_("Cx Program Change"), MIDI_M_PGM_CHANGE);
  if (m_ReceiverType == MIDI_RECV_MANUAL) {
    m_eventtype->Append(_("9x Note without Velocity"), MIDI_M_NOTE_NO_VELOCITY);
    m_eventtype->Append(
      _("9x Note short octave at low key"), MIDI_M_NOTE_SHORT_OCTAVE);
    m_eventtype->Append(_("9x Note without map"), MIDI_M_NOTE_NORMAL);
  }

  if (m_ReceiverType != MIDI_RECV_MANUAL) {
    m_eventtype->Append(_("RPN"), MIDI_M_RPN);
    m_eventtype->Append(_("NRPN"), MIDI_M_NRPN);
    m_eventtype->Append(_("Cx Program Change Range"), MIDI_M_PGM_RANGE);
  }
  if (
    m_ReceiverType != MIDI_RECV_MANUAL
    && m_ReceiverType != MIDI_RECV_ENCLOSURE) {
    m_eventtype->Append(_("RPN On Toggle"), MIDI_M_RPN_ON);
    m_eventtype->Append(_("RPN Off Toggle"), MIDI_M_RPN_OFF);
    m_eventtype->Append(_("RPN On/Off Toggle"), MIDI_M_RPN_ON_OFF);
    m_eventtype->Append(_("NRPN On Toggle"), MIDI_M_NRPN_ON);
    m_eventtype->Append(_("NRPN Off Toggle"), MIDI_M_NRPN_OFF);
    m_eventtype->Append(_("NRPN On/Off Toggle"), MIDI_M_NRPN_ON_OFF);
    m_eventtype->Append(_("RPN Range"), MIDI_M_RPN_RANGE);
    m_eventtype->Append(_("NRPN Range"), MIDI_M_NRPN_RANGE);
  }

  if (
    m_ReceiverType != MIDI_RECV_MANUAL
    && m_ReceiverType != MIDI_RECV_ENCLOSURE) {
    m_eventtype->Append(_("Ctrl Change Bitfield"), MIDI_M_CTRL_BIT);
    m_eventtype->Append(
      _("Bx Ctrl Change Fixed Value"), MIDI_M_CTRL_CHANGE_FIXED);
    m_eventtype->Append(
      _("Bx Ctrl Change Fixed On Value Toggle"), MIDI_M_CTRL_CHANGE_FIXED_ON);
    m_eventtype->Append(
      _("Bx Ctrl Change Fixed Off Value Toggle"), MIDI_M_CTRL_CHANGE_FIXED_OFF);
    m_eventtype->Append(
      _("Bx Ctrl Change Fixed On/Off Value Toggle"),
      MIDI_M_CTRL_CHANGE_FIXED_ON_OFF);
    m_eventtype->Append(_("Sys Ex Johannus 9 bytes"), MIDI_M_SYSEX_JOHANNUS_9);
    m_eventtype->Append(
      _("Sys Ex Johannus 11 bytes"), MIDI_M_SYSEX_JOHANNUS_11);
    m_eventtype->Append(_("Sys Ex Viscount"), MIDI_M_SYSEX_VISCOUNT);
    m_eventtype->Append(
      _("Sys Ex Viscount Toggle"), MIDI_M_SYSEX_VISCOUNT_TOGGLE);
    m_eventtype->Append(
      _("Sys Ex Rogers Stop Change"), MIDI_M_SYSEX_RODGERS_STOP_CHANGE);
    m_eventtype->Append(
      _("Sys Ex Ahlborn-Galanti"), MIDI_M_SYSEX_AHLBORN_GALANTI);
    m_eventtype->Append(
      _("Sys Ex Ahlborn-Galanti Toggle"), MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE);
  }

  m_current = 0;
  if (!m_midi.GetEventCount())
    m_midi.AddNewEvent();

  LoadEvent();

  topSizer->Fit(this);
}

GOMidiEventRecvTab::~GOMidiEventRecvTab() { StopListen(); }

void GOMidiEventRecvTab::RegisterMIDIListener(GOMidi *midi) {
  if (midi)
    m_listener.Register(midi);
}

bool GOMidiEventRecvTab::TransferDataFromWindow() {
  StoreEvent();

  bool empty_event;
  do {
    empty_event = false;
    for (unsigned i = 0; i < m_midi.GetEventCount(); i++)
      if (m_midi.GetEvent(i).type == MIDI_M_NONE) {
        m_midi.DeleteEvent(i);
        empty_event = true;
      }
  } while (empty_event);

  if (m_original->RenewFrom(m_midi))
    GOModificationProxy::OnIsModifiedChanged(true);
  return true;
}

void GOMidiEventRecvTab::OnTypeChange(wxCommandEvent &event) {
  GOMidiReceiverMessageType type = m_eventtype->GetCurrentSelection();
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
  if (m_original->HasLowerLimit(type)) {
    m_LowValue->Enable();
    m_LowValue->SetRange(0, m_original->LowerValueLimit(type));
  } else
    m_LowValue->Disable();
  if (m_original->HasUpperLimit(type)) {
    m_HighValue->Enable();
    m_HighValue->SetRange(0, m_original->UpperValueLimit(type));
  } else
    m_HighValue->Disable();
  if (m_original->HasKey(type)) {
    m_data->Enable();
    if (m_ReceiverType == MIDI_RECV_MANUAL)
      m_data->SetRange(-35, 35);
    else
      m_data->SetRange(0, m_original->KeyLimit(type));
  } else
    m_data->Disable();

  if (m_ReceiverType == MIDI_RECV_MANUAL) {
    m_LowValueLabel->SetLabel(_("L&owest velocity:"));
    m_HighValueLabel->SetLabel(_("H&ighest velocity:"));
    m_DataLabel->SetLabel(_("&Transpose:"));
  } else {
    if (type == MIDI_M_CTRL_BIT)
      m_LowValueLabel->SetLabel(_("&Bit number:"));
    else if (
      type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_CTRL_CHANGE_FIXED_OFF
      || type == MIDI_M_SYSEX_VISCOUNT || type == MIDI_M_SYSEX_AHLBORN_GALANTI
      || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE)
      m_LowValueLabel->SetLabel(_("&Off value:"));
    else if (type == MIDI_M_PGM_RANGE)
      m_LowValueLabel->SetLabel(_("&Lower PGM number:"));
    else if (type == MIDI_M_RPN_RANGE)
      m_LowValueLabel->SetLabel(_("&Off RPN number:"));
    else if (type == MIDI_M_NRPN_RANGE)
      m_LowValueLabel->SetLabel(_("&Off NRPN number:"));
    else if (type == MIDI_M_SYSEX_VISCOUNT_TOGGLE)
      m_LowValueLabel->SetLabel(_("&Value:"));
    else if (type == MIDI_M_SYSEX_JOHANNUS_11)
      m_LowValueLabel->SetLabel(_("L&ower bank:"));
    else if (type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE)
      m_LowValueLabel->SetLabel(_("&Stop number:"));
    else
      m_LowValueLabel->SetLabel(_("L&ower limit:"));

    if (
      type == MIDI_M_CTRL_CHANGE_FIXED || type == MIDI_M_CTRL_CHANGE_FIXED_ON
      || type == MIDI_M_SYSEX_VISCOUNT || type == MIDI_M_SYSEX_AHLBORN_GALANTI
      || type == MIDI_M_SYSEX_AHLBORN_GALANTI_TOGGLE)
      m_HighValueLabel->SetLabel(_("&On value:"));
    else if (type == MIDI_M_PGM_RANGE)
      m_HighValueLabel->SetLabel(_("&Upper PGM number:"));
    else if (type == MIDI_M_RPN_RANGE)
      m_HighValueLabel->SetLabel(_("&On RPN number:"));
    else if (type == MIDI_M_NRPN_RANGE)
      m_HighValueLabel->SetLabel(_("&On NRPN number:"));
    else if (type == MIDI_M_SYSEX_JOHANNUS_11)
      m_HighValueLabel->SetLabel(_("&Upper bank:"));
    else
      m_HighValueLabel->SetLabel(_("&Upper limit:"));

    switch (type) {
    case MIDI_M_CTRL_CHANGE:
    case MIDI_M_CTRL_BIT:
    case MIDI_M_CTRL_CHANGE_ON:
    case MIDI_M_CTRL_CHANGE_OFF:
    case MIDI_M_CTRL_CHANGE_ON_OFF:
    case MIDI_M_CTRL_CHANGE_FIXED:
    case MIDI_M_CTRL_CHANGE_FIXED_ON:
    case MIDI_M_CTRL_CHANGE_FIXED_OFF:
    case MIDI_M_CTRL_CHANGE_FIXED_ON_OFF:
      m_DataLabel->SetLabel(_("&Controller-No:"));
      break;

    case MIDI_M_RPN:
    case MIDI_M_NRPN:
    case MIDI_M_RPN_ON:
    case MIDI_M_RPN_OFF:
    case MIDI_M_RPN_ON_OFF:
    case MIDI_M_NRPN_ON:
    case MIDI_M_NRPN_OFF:
    case MIDI_M_NRPN_ON_OFF:
      m_DataLabel->SetLabel(_("&Parameter-No:"));
      break;

    case MIDI_M_RPN_RANGE:
    case MIDI_M_NRPN_RANGE:
      m_DataLabel->SetLabel(_("&Value:"));
      break;

    case MIDI_M_SYSEX_RODGERS_STOP_CHANGE:
      m_DataLabel->SetLabel(_("&Device number:"));
      break;

    default:
      m_DataLabel->SetLabel(_("&Data:"));
    }
  }
  Layout();
}

void GOMidiEventRecvTab::LoadEvent() {
  m_eventno->Clear();
  for (unsigned i = 0; i < m_midi.GetEventCount(); i++) {
    wxString buffer;
    wxString device;
    if (m_midi.GetEvent(i).deviceId == 0)
      device = _("Any device");
    else
      device = m_MidiMap.GetDeviceLogicalNameById(m_midi.GetEvent(i).deviceId);
    buffer.Printf(_("%d (%s)"), i + 1, device.c_str());
    m_eventno->Append(buffer);
  }
  m_eventno->Select(m_current);
  if (m_midi.GetEventCount() > 1)
    m_delete->Enable();
  else
    m_delete->Disable();

  GOMidiReceiverEventPattern &e = m_midi.GetEvent(m_current);

  m_eventtype->SetCurrentSelection(e.type);

  wxCommandEvent event;
  OnTypeChange(event);

  // Select current device
  m_device->SetSelection(0);
  if (e.deviceId > 0) {
    const wxString &eventDeviceLogicalName
      = m_MidiMap.GetDeviceLogicalNameById(e.deviceId);

    for (unsigned i = 1; i < m_device->GetCount(); i++) {
      const wxString &deviceLogicalName = m_device->GetString(i);

      if (deviceLogicalName == eventDeviceLogicalName)
        m_device->SetSelection(i);
    }
  }

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

GOMidiReceiverEventPattern GOMidiEventRecvTab::GetCurrentEvent() {
  GOMidiReceiverEventPattern e;
  if (m_device->GetSelection() == 0)
    e.deviceId = 0;
  else
    e.deviceId
      = m_MidiMap.GetDeviceIdByLogicalName(m_device->GetStringSelection());

  e.type = m_eventtype->GetCurrentSelection();
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
  return e;
}

void GOMidiEventRecvTab::StoreEvent() {
  m_midi.GetEvent(m_current) = GetCurrentEvent();
}

void GOMidiEventRecvTab::OnNewClick(wxCommandEvent &event) {
  StoreEvent();
  m_current = m_midi.AddNewEvent();
  LoadEvent();
}

void GOMidiEventRecvTab::OnDeleteClick(wxCommandEvent &event) {
  m_midi.DeleteEvent(m_current);
  m_current = 0;
  LoadEvent();
}

void GOMidiEventRecvTab::OnEventChange(wxCommandEvent &event) {
  StoreEvent();
  m_current = m_eventno->GetSelection();
  LoadEvent();
}

void GOMidiEventRecvTab::OnListenSimpleClick(wxCommandEvent &event) {
  if (m_ListenSimple->GetValue())
    StartListen(false);
  else
    StopListen();
}

void GOMidiEventRecvTab::OnListenAdvancedClick(wxCommandEvent &event) {
  if (m_ListenAdvanced->GetValue())
    StartListen(true);
  else
    StopListen();
}

void GOMidiEventRecvTab::OnTimer(wxTimerEvent &event) {
  if (m_ListenState == 2) {
    wxString label;
    switch (m_ReceiverType) {
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
  } else if (m_ListenState == 3)
    DetectEvent();
}

void GOMidiEventRecvTab::OnMidiEvent(const GOMidiEvent &event) {
  switch (event.GetMidiType()) {
  case GOMidiEvent::MIDI_NOTE:
  case GOMidiEvent::MIDI_CTRL_CHANGE:
  case GOMidiEvent::MIDI_PGM_CHANGE:
  case GOMidiEvent::MIDI_RPN:
  case GOMidiEvent::MIDI_NRPN:
  case GOMidiEvent::MIDI_SYSEX_JOHANNUS_9:
  case GOMidiEvent::MIDI_SYSEX_JOHANNUS_11:
  case GOMidiEvent::MIDI_SYSEX_VISCOUNT:
  case GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE:
  case GOMidiEvent::MIDI_SYSEX_AHLBORN_GALANTI:
    break;

  default:
    return;
  }

  if (m_ListenState == 1) {
    m_OnList.push_back(event);
    DetectEvent();
  } else if (m_ListenState == 2) {
    m_Timer.Stop();
    m_Timer.Start(1000, true);
    m_OnList.push_back(event);
  } else if (m_ListenState == 3) {
    m_OffList.push_back(event);
    m_Timer.Stop();
    m_Timer.Start(1000, true);
  }
}

void GOMidiEventRecvTab::StartListen(bool type) {
  this->SetCursor(wxCursor(wxCURSOR_WAIT));
  m_listener.SetCallback(this);
  if (!type)
    m_ListenAdvanced->Disable();
  if (type)
    m_ListenSimple->Disable();
  m_OnList.clear();
  m_OffList.clear();
  if (!type) {
    m_ListenInstructions->SetLabel(_("Please press the MIDI element"));
    m_ListenState = 1;
  } else {
    m_ListenState = 2;
    wxString label;
    switch (m_ReceiverType) {
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

void GOMidiEventRecvTab::StopListen() {
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

bool GOMidiEventRecvTab::SimilarEvent(
  const GOMidiEvent &e1, const GOMidiEvent &e2) {
  if (e1.GetDevice() != e2.GetDevice())
    return false;
  if (e1.GetMidiType() != e2.GetMidiType())
    return false;
  if (e1.GetChannel() != e2.GetChannel())
    return false;

  if (e1.GetMidiType() == GOMidiEvent::MIDI_PGM_CHANGE)
    return true;
  if (e1.GetMidiType() == GOMidiEvent::MIDI_SYSEX_VISCOUNT)
    return true;
  if (e1.GetMidiType() == GOMidiEvent::MIDI_SYSEX_AHLBORN_GALANTI)
    return true;
  if (e1.GetKey() == e2.GetKey()) {
    if (e1.GetMidiType() != GOMidiEvent::MIDI_NOTE)
      return true;
  }
  return false;
}

void GOMidiEventRecvTab::DetectEvent() {
  // We will fill the event pattern here
  GOMidiReceiverEventPattern &e = m_midi.GetEvent(m_current);
  // whether we has already filled the event
  bool hasFilled = false;

  if (m_ListenState == 3) {
    for (unsigned i = 0; i < m_OnList.size(); i++) {
      if (i + 1 < m_OnList.size()) {
        if (SimilarEvent(m_OnList[i], m_OnList[i + 1]))
          continue;
      }

      GOMidiEvent on = m_OnList[i];

      for (unsigned j = 0; j < m_OffList.size(); j++) {
        if (j + 1 < m_OffList.size()) {
          if (SimilarEvent(m_OffList[j], m_OffList[j + 1])) {
            if (m_OffList[j].GetMidiType() != GOMidiEvent::MIDI_NOTE)
              continue;
          }
        }

        GOMidiEvent off = m_OffList[j];

        if (on.GetDevice() != off.GetDevice())
          continue;
        if (on.GetChannel() != off.GetChannel())
          continue;
        if (on.GetMidiType() != off.GetMidiType())
          continue;

        const GOMidiEvent::MidiType eventMidiType = on.GetMidiType();
        bool is_range = false;

        if (m_ReceiverType == MIDI_RECV_MANUAL) {
          if (eventMidiType != GOMidiEvent::MIDI_NOTE)
            continue;
          e.type = MIDI_M_NOTE;
          e.deviceId = on.GetDevice();
          e.channel = on.GetChannel();
          e.low_key = on.GetKey();
          e.high_key = off.GetKey();
          e.low_value = std::min(on.GetValue(), off.GetValue());
          e.high_value = 127;
          e.debounce_time = 0;
          if ((on.GetKey() % 12) == 4) {
            e.type = MIDI_M_NOTE_SHORT_OCTAVE;
            e.low_key -= 4;
          }
          hasFilled = true;
        } else if (
          (m_ReceiverType == MIDI_RECV_BUTTON
           || m_ReceiverType == MIDI_RECV_DRAWSTOP)
          && (eventMidiType == GOMidiEvent::MIDI_CTRL_CHANGE || eventMidiType == GOMidiEvent::MIDI_NOTE)
          && on.GetKey() != off.GetKey() && on.GetValue() == off.GetValue()) {
          const GOMidiReceiverMessageType onType
            = (eventMidiType == GOMidiEvent::MIDI_CTRL_CHANGE)
            ? MIDI_M_CTRL_CHANGE_FIXED_ON
            : MIDI_M_NOTE_FIXED_ON;
          const GOMidiReceiverMessageType offType
            = (eventMidiType == GOMidiEvent::MIDI_CTRL_CHANGE)
            ? MIDI_M_CTRL_CHANGE_FIXED_OFF
            : MIDI_M_NOTE_FIXED_OFF;
          GOMidiReceiverEventPattern ep;

          // different keys for On and Off
          ep.deviceId = on.GetDevice();
          ep.channel = on.GetChannel();
          ep.low_value = on.GetValue();
          ep.high_value = on.GetValue();
          ep.debounce_time = 0;
          e = ep;
          e.type = onType;
          e.key = on.GetKey();

          GOMidiReceiverEventPattern *pOffEvent = nullptr;

          // find among existing events
          for (unsigned lk = m_midi.GetEventCount(), k = 0; k < lk; k++)
            if ((int)k != m_current) {
              GOMidiReceiverEventPattern &eK = m_midi.GetEvent(k);

              if (
                eK.channel == e.channel && eK.deviceId == e.deviceId
                && (eK.type == onType || eK.type == offType)) {
                pOffEvent = &eK;
                break;
              }
            }

          // Add a new event it has not been found
          if (!pOffEvent)
            pOffEvent = &m_midi.GetEvent(m_midi.AddNewEvent());
          *pOffEvent = ep;
          pOffEvent->type = offType;
          pOffEvent->key = off.GetKey();
          hasFilled = true;
        } else {
          if (
            on.GetValue() == off.GetValue() && on.GetKey() != off.GetKey()
            && (eventMidiType == GOMidiEvent::MIDI_RPN || eventMidiType == GOMidiEvent::MIDI_NRPN || eventMidiType == GOMidiEvent::MIDI_CTRL_CHANGE)) {
            if (m_ReceiverType == MIDI_RECV_ENCLOSURE)
              is_range = false;
            else
              is_range = true;
          } else if (eventMidiType == GOMidiEvent::MIDI_PGM_CHANGE) {
            is_range = true;
          }
          if (on.GetKey() != off.GetKey() && !is_range)
            continue;
          if (m_ReceiverType == MIDI_RECV_ENCLOSURE) {
            unsigned low = off.GetValue();
            unsigned high = on.GetValue();
            int key = on.GetKey();
            switch (eventMidiType) {
            case GOMidiEvent::MIDI_CTRL_CHANGE:
              e.type = MIDI_M_CTRL_CHANGE;
              break;
            case GOMidiEvent::MIDI_RPN:
              e.type = MIDI_M_RPN;
              break;
            case GOMidiEvent::MIDI_NRPN:
              e.type = MIDI_M_NRPN;
              break;
            case GOMidiEvent::MIDI_PGM_CHANGE:
              e.type = MIDI_M_PGM_RANGE;
              low = off.GetKey();
              high = on.GetKey();
              key = -1;
              break;
            default:
              continue;
            }
            e.deviceId = on.GetDevice();
            e.channel = on.GetChannel();
            e.key = key;
            e.low_key = 0;
            e.high_key = 0;
            e.high_value = high;
            e.low_value = low;
            hasFilled = true;
          } else {
            unsigned low = 0;
            unsigned high = 1;
            int key = on.GetKey();
            switch (eventMidiType) {
            case GOMidiEvent::MIDI_NOTE:
              e.type = MIDI_M_NOTE;
              if (on.GetValue() > 0 && off.GetValue() > 0)
                e.type = MIDI_M_NOTE_ON;
              if (on.GetValue() == 0 && off.GetValue() == 0)
                e.type = MIDI_M_NOTE_OFF;
              break;
            case GOMidiEvent::MIDI_CTRL_CHANGE:
              e.type = MIDI_M_CTRL_CHANGE;
              if (on.GetValue() == off.GetValue())
                e.type = on.GetValue() > 0 ? MIDI_M_CTRL_CHANGE_ON
                                           : MIDI_M_CTRL_CHANGE_OFF;
              for (unsigned k = 0; k < 7; k++)
                if (on.GetValue() == off.GetValue() + (1 << k)) {
                  e.type = MIDI_M_CTRL_BIT;
                  low = k;
                }
              if (
                off.GetValue() != 0 && (on.GetValue() & 64)
                && (on.GetValue() & 63) == off.GetValue()) {
                if (e.type != MIDI_M_CTRL_BIT || on.GetKey() >= 60) {
                  high = on.GetValue();
                  low = off.GetValue();
                  e.type = MIDI_M_CTRL_CHANGE_FIXED;
                }
              }
              break;
            case GOMidiEvent::MIDI_PGM_CHANGE:
              if (on.GetKey() == off.GetKey())
                e.type = MIDI_M_PGM_CHANGE;
              else {
                e.type = MIDI_M_PGM_RANGE;
                low = off.GetKey();
                high = on.GetKey();
                key = -1;
              }
              break;
            case GOMidiEvent::MIDI_RPN:
              if (is_range) {
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
            case GOMidiEvent::MIDI_NRPN:
              if (is_range) {
                e.type = MIDI_M_NRPN_RANGE;
                key = on.GetValue();
                low = off.GetKey();
                high = on.GetKey();
                break;
              }
              e.type = MIDI_M_NRPN;
              if (on.GetValue() == off.GetValue())
                e.type = on.GetValue() > 0 ? MIDI_M_NRPN_ON : MIDI_M_NRPN_OFF;
              break;
            case GOMidiEvent::MIDI_SYSEX_JOHANNUS_9:
              e.type = MIDI_M_SYSEX_JOHANNUS_9;
              break;
            case GOMidiEvent::MIDI_SYSEX_JOHANNUS_11:
              e.type = MIDI_M_SYSEX_JOHANNUS_11;
              high = 0x7f;
              break;
            case GOMidiEvent::MIDI_SYSEX_VISCOUNT:
              if (on.GetValue() == off.GetValue()) {
                low = off.GetValue();
                e.type = MIDI_M_SYSEX_VISCOUNT_TOGGLE;
              } else {
                low = off.GetValue();
                high = on.GetValue();
                e.type = MIDI_M_SYSEX_VISCOUNT;
              }
              break;
            case GOMidiEvent::MIDI_SYSEX_RODGERS_STOP_CHANGE:
              for (unsigned i = 0; i < m_original->LowerValueLimit(
                                     MIDI_M_SYSEX_RODGERS_STOP_CHANGE);
                   i++) {
                if (
                  GORodgersGetBit(i, off.GetKey(), off.GetData())
                    == MIDI_BIT_STATE::MIDI_BIT_CLEAR
                  && GORodgersGetBit(i, on.GetKey(), on.GetData())
                    == MIDI_BIT_STATE::MIDI_BIT_SET) {
                  key = on.GetChannel();
                  low = i;
                  e.type = MIDI_M_SYSEX_RODGERS_STOP_CHANGE;
                }
              }
              break;
            case GOMidiEvent::MIDI_SYSEX_AHLBORN_GALANTI:
              low = off.GetValue();
              high = on.GetValue();
              e.type = MIDI_M_SYSEX_AHLBORN_GALANTI;
              break;

            default:
              continue;
            }
            e.deviceId = on.GetDevice();
            e.channel = on.GetChannel();
            e.key = key;
            e.low_key = 0;
            e.high_key = 0;
            e.low_value = low;
            e.high_value = high;
            e.debounce_time = 0;
            hasFilled = true;
          }
        }
        if (hasFilled)
          break;
      }
      if (hasFilled)
        break;
    }
  }

  if (!hasFilled) {
    // could not detect complex midi setup. Detect a simple one
    GOMidiEvent &event = m_OnList[0];
    unsigned low_value = m_ReceiverType == MIDI_RECV_MANUAL ? 1 : 0;
    unsigned high_value = (m_ReceiverType == MIDI_RECV_MANUAL
                           || m_ReceiverType == MIDI_RECV_ENCLOSURE)
      ? 127
      : 1;
    switch (event.GetMidiType()) {
    case GOMidiEvent::MIDI_NOTE:
      e.type = MIDI_M_NOTE;
      break;
    case GOMidiEvent::MIDI_CTRL_CHANGE:
      e.type = MIDI_M_CTRL_CHANGE;
      break;
    case GOMidiEvent::MIDI_PGM_CHANGE:
      e.type = MIDI_M_PGM_CHANGE;
      break;
    case GOMidiEvent::MIDI_RPN:
      e.type = MIDI_M_RPN;
      break;
    case GOMidiEvent::MIDI_NRPN:
      e.type = MIDI_M_NRPN;
      break;
    case GOMidiEvent::MIDI_SYSEX_JOHANNUS_9:
      e.type = MIDI_M_SYSEX_JOHANNUS_9;
      break;
    case GOMidiEvent::MIDI_SYSEX_JOHANNUS_11:
      e.type = MIDI_M_SYSEX_JOHANNUS_11;
      high_value = 127;
      break;
    case GOMidiEvent::MIDI_SYSEX_VISCOUNT:
      e.type = MIDI_M_SYSEX_VISCOUNT_TOGGLE;
      low_value = event.GetValue();
      break;
    case GOMidiEvent::MIDI_SYSEX_AHLBORN_GALANTI:
      e.type = MIDI_M_SYSEX_AHLBORN_GALANTI;
      if (((event.GetValue() >> 7) & 0x0f) < 8) {
        high_value = event.GetValue();
        low_value = high_value + (6 << 7);
      } else {
        low_value = event.GetValue();
        high_value = low_value - (6 << 7);
      }
      break;

    default:
      e.type = MIDI_M_NONE;
    }
    e.deviceId = event.GetDevice();
    e.channel = event.GetChannel();
    if (m_ReceiverType != MIDI_RECV_MANUAL)
      e.key = event.GetKey();
    e.low_key = 0;
    e.high_key = 127;
    e.low_value = low_value;
    e.high_value = high_value;
    e.debounce_time = 0;
  }

  LoadEvent();
  StopListen();
}
