/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiEventSendTab.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "config/GOConfig.h"
#include "dialogs/common/GOTabbedDialog.h"

#include "GOMidiEventRecvTab.h"

BEGIN_EVENT_TABLE(GOMidiEventSendTab, wxPanel)
EVT_BUTTON(ID_EVENT_NEW, GOMidiEventSendTab::OnNewClick)
EVT_BUTTON(ID_EVENT_DELETE, GOMidiEventSendTab::OnDeleteClick)
EVT_BUTTON(ID_COPY, GOMidiEventSendTab::OnCopyClick)
EVT_CHOICE(ID_EVENT_NO, GOMidiEventSendTab::OnEventChange)
EVT_CHOICE(ID_EVENT, GOMidiEventSendTab::OnTypeChange)
END_EVENT_TABLE()

GOMidiEventSendTab::GOMidiEventSendTab(
  GOTabbedDialog *pDlg,
  const wxString &label,
  GOMidiSender *event,
  GOMidiEventRecvTab *recv,
  GOConfig &config)
  : GODialogTab(pDlg, "Send", label),
    m_MidiIn(config.m_MidiIn),
    m_MidiOut(config.m_MidiOut),
    m_MidiMap(config.GetMidiMap()),
    m_original(event),
    m_recv(recv),
    m_midi(*event) {
  wxSizer *const topSizer = new wxBoxSizer(wxVERTICAL);
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
  m_eventtype = new GOChoice<GOMidiSenderMessageType>(this, ID_EVENT);
  grid->Add(m_eventtype, wxGBPosition(2, 1), wxGBSpan(1, 4), wxEXPAND);

  grid->Add(
    new wxStaticText(this, wxID_ANY, _("&Channel:")),
    wxGBPosition(3, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_channel = new wxChoice(this, ID_CHANNEL);
  grid->Add(m_channel, wxGBPosition(3, 1));

  m_KeyLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_KeyLabel,
    wxGBPosition(4, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_key = new wxSpinCtrl(
    this,
    ID_KEY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_key, wxGBPosition(4, 1));

  m_noteOff = new wxCheckBox(
    this,
    ID_NOTE_OFF,
    wxT("Use Note Off (0x8X)"),
    wxDefaultPosition,
    wxDefaultSize);
  grid->Add(m_noteOff, wxGBPosition(4, 2));

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

  m_StartLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_StartLabel,
    wxGBPosition(6, 0),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  m_StartValue = new wxSpinCtrl(
    this,
    ID_START,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_StartValue, wxGBPosition(6, 1));
  m_LengthLabel = new wxStaticText(this, wxID_ANY, wxT(""));
  grid->Add(
    m_LengthLabel,
    wxGBPosition(6, 2),
    wxDefaultSpan,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT,
    5);
  m_LengthValue = new wxSpinCtrl(
    this,
    ID_LENGTH,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    0,
    127);
  grid->Add(m_LengthValue, wxGBPosition(6, 3));

  grid->AddGrowableCol(4, 1);
  topSizer->Add(grid, 0, wxEXPAND | wxALL, 5);

  box = new wxBoxSizer(wxHORIZONTAL);
  m_copy = new wxButton(this, ID_COPY, _("&Copy current receive event"));
  if (!m_recv)
    m_copy->Disable();
  box->Add(m_copy, 0, wxALL, 5);
  topSizer->Add(box, 0, wxEXPAND | wxALL, 5);

  SetSizer(topSizer);

  const GOPortsConfig &portsConfig = config.GetMidiPortsConfig();

  m_device->Append(_("Any device"));
  for (const GOMidiDeviceConfig *pDevConf : m_MidiOut)
    if (
      portsConfig.IsEnabled(pDevConf->m_PortName, pDevConf->m_ApiName)
      && pDevConf->m_IsEnabled)
      m_device->Append(pDevConf->m_LogicalName);

  for (unsigned int i = 1; i <= 16; i++)
    m_channel->Append(wxString::Format(wxT("%d"), i));
  ;

  m_eventtype->Append(_("(none)"), MIDI_S_NONE);
  if (
    m_midi.GetType() == MIDI_SEND_MANUAL
    || m_midi.GetType() == MIDI_SEND_BUTTON)
    m_eventtype->Append(_("9x Note"), MIDI_S_NOTE);
  if (m_midi.GetType() == MIDI_SEND_MANUAL)
    m_eventtype->Append(_("9x Note without Velocity"), MIDI_S_NOTE_NO_VELOCITY);
  if (m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(_("9x Note On"), MIDI_S_NOTE_ON);
    m_eventtype->Append(_("9x Note Off"), MIDI_S_NOTE_OFF);
  }
  if (
    m_midi.GetType() == MIDI_SEND_ENCLOSURE
    || m_midi.GetType() == MIDI_SEND_BUTTON)
    m_eventtype->Append(_("Bx Controller"), MIDI_S_CTRL);
  if (m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(_("Bx Controller On"), MIDI_S_CTRL_ON);
    m_eventtype->Append(_("Bx Controller Off"), MIDI_S_CTRL_OFF);
  }
  if (m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(_("Cx Program Change On"), MIDI_S_PGM_ON);
    m_eventtype->Append(_("Cx Program Change Off"), MIDI_S_PGM_OFF);
  }
  if (
    m_midi.GetType() == MIDI_SEND_ENCLOSURE
    || m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(_("RPN"), MIDI_S_RPN);
    m_eventtype->Append(_("NRPN"), MIDI_S_NRPN);
    m_eventtype->Append(_("Cx Program Change Range"), MIDI_S_PGM_RANGE);
  }
  if (m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(_("RPN On"), MIDI_S_RPN_ON);
    m_eventtype->Append(_("RPN Off"), MIDI_S_RPN_OFF);
    m_eventtype->Append(_("NRPN On"), MIDI_S_NRPN_ON);
    m_eventtype->Append(_("NRPN Off"), MIDI_S_NRPN_OFF);
    m_eventtype->Append(_("RPN Range"), MIDI_S_RPN_RANGE);
    m_eventtype->Append(_("NRPN Range"), MIDI_S_NRPN_RANGE);
  }
  if (
    m_midi.GetType() == MIDI_SEND_LABEL || m_midi.GetType() == MIDI_SEND_BUTTON
    || m_midi.GetType() == MIDI_SEND_ENCLOSURE) {
    m_eventtype->Append(_("SYSEX Hauptwerk 32 Byte LCD Value"), MIDI_S_HW_LCD);
    m_eventtype->Append(
      _("SYSEX Hauptwerk 16 Byte String Value"), MIDI_S_HW_STRING);
  }
  m_eventtype->Append(
    _("SYSEX Hauptwerk 32 Byte LCD Name"), MIDI_S_HW_NAME_LCD);
  m_eventtype->Append(
    _("SYSEX Hauptwerk 16 Byte String Name"), MIDI_S_HW_NAME_STRING);
  if (m_midi.GetType() == MIDI_SEND_BUTTON) {
    m_eventtype->Append(
      _("SYSEX Rogers Stop Change"), MIDI_S_RODGERS_STOP_CHANGE);
  }

  m_current = 0;
  if (!m_midi.GetEventCount())
    m_midi.AddNewEvent();

  LoadEvent();

  topSizer->Fit(this);
}

GOMidiEventSendTab::~GOMidiEventSendTab() {}

bool GOMidiEventSendTab::TransferDataFromWindow() {
  // save the current event being edited
  StoreEvent();

  // Delete empty events.
  bool empty_event;
  do {
    empty_event = false;
    for (unsigned i = 0; i < m_midi.GetEventCount(); i++)
      if (m_midi.GetEvent(i).type == MIDI_S_NONE) {
        m_midi.DeleteEvent(i);
        empty_event = true;
      }
  } while (empty_event);
  // The event with index 0 is also deleted so the dialog can't be used more

  if (m_original->RenewFrom(m_midi))
    OnIsModifiedChanged(true);
  return true;
}

void GOMidiEventSendTab::OnTypeChange(wxCommandEvent &event) {
  GOMidiSenderMessageType type = m_eventtype->GetCurrentSelection();
  if (m_original->HasChannel(type))
    m_channel->Enable();
  else
    m_channel->Disable();
  if (m_original->HasKey(type)) {
    m_key->Enable();
    m_key->SetRange(0, m_original->KeyLimit(type));
  } else
    m_key->Disable();
  if (m_original->IsNote(type))
    m_noteOff->Enable();
  else
    m_noteOff->Disable();
  if (m_original->HasLowValue(type)) {
    m_LowValue->Enable();
    m_LowValue->SetRange(0, m_original->LowValueLimit(type));
  } else
    m_LowValue->Disable();
  if (m_original->HasHighValue(type)) {
    m_HighValue->Enable();
    m_HighValue->SetRange(0, m_original->HighValueLimit(type));
  } else
    m_HighValue->Disable();
  if (m_original->HasStart(type)) {
    m_StartValue->Enable();
    m_StartValue->SetRange(0, m_original->StartLimit(type));
  } else
    m_StartValue->Disable();
  if (m_original->HasLength(type)) {
    m_LengthValue->Enable();
    m_LengthValue->SetRange(0, m_original->LengthLimit(type));
  } else
    m_LengthValue->Disable();
  if (type == MIDI_S_HW_LCD || type == MIDI_S_HW_NAME_LCD)
    m_LowValueLabel->SetLabel(_("&Color:"));
  else if (type == MIDI_S_PGM_RANGE)
    m_LowValueLabel->SetLabel(_("&Lower PGM number:"));
  else if (type == MIDI_S_RPN_RANGE)
    m_LowValueLabel->SetLabel(_("&Off RPN number:"));
  else if (type == MIDI_S_NRPN_RANGE)
    m_LowValueLabel->SetLabel(_("&Off NRPN number:"));
  else if (type == MIDI_S_RODGERS_STOP_CHANGE)
    m_LowValueLabel->SetLabel(_("&Stop number:"));
  else
    m_LowValueLabel->SetLabel(_("&Off Value:"));
  if (type == MIDI_S_PGM_RANGE)
    m_HighValueLabel->SetLabel(_("&Upper PGM number:"));
  else if (type == MIDI_S_RPN_RANGE)
    m_HighValueLabel->SetLabel(_("&On RPN number:"));
  else if (type == MIDI_S_NRPN_RANGE)
    m_HighValueLabel->SetLabel(_("&On NRPN number:"));
  else
    m_HighValueLabel->SetLabel(_("&On value:"));
  switch (type) {
  case MIDI_S_NOTE_ON:
  case MIDI_S_NOTE_OFF:
  case MIDI_S_NOTE:
    m_KeyLabel->SetLabel(_("&MIDI-note:"));
    break;

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

  case MIDI_S_HW_NAME_STRING:
  case MIDI_S_HW_NAME_LCD:
  case MIDI_S_HW_STRING:
  case MIDI_S_HW_LCD:
    m_KeyLabel->SetLabel(_("&ID:"));
    break;

  case MIDI_S_RPN_RANGE:
  case MIDI_S_NRPN_RANGE:
    m_KeyLabel->SetLabel(_("&Value:"));
    break;

  case MIDI_S_RODGERS_STOP_CHANGE:
    m_KeyLabel->SetLabel(_("&Device number:"));
    break;

  default:
    m_KeyLabel->SetLabel(_("&CTRL/PGM:"));
  }
  m_StartLabel->SetLabel(_("&Start-Position:"));
  m_LengthLabel->SetLabel(_("&Length:"));
  Layout();
}

void GOMidiEventSendTab::LoadEvent() {
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

  GOMidiSenderEventPattern &e = m_midi.GetEvent(m_current);

  m_eventtype->SetCurrentSelection(e.type);

  wxCommandEvent event;
  OnTypeChange(event);

  m_device->SetSelection(0);
  for (unsigned i = 1; i < m_device->GetCount(); i++)
    if (
      m_MidiMap.GetDeviceLogicalNameById(e.deviceId) == m_device->GetString(i))
      m_device->SetSelection(i);

  m_channel->SetSelection(e.channel > 0 && e.channel <= 16 ? e.channel - 1 : 0);
  m_key->SetValue(e.key);
  m_LowValue->SetValue(e.low_value);
  m_HighValue->SetValue(e.high_value);
  m_StartValue->SetValue(e.start);
  m_LengthValue->SetValue(e.length);
  m_noteOff->SetValue(e.useNoteOff);
}

void GOMidiEventSendTab::StoreEvent() {
  GOMidiSenderEventPattern &e = m_midi.GetEvent(m_current);
  if (m_device->GetSelection() == 0)
    e.deviceId = 0;
  else
    e.deviceId
      = m_MidiMap.GetDeviceIdByLogicalName(m_device->GetStringSelection());

  e.type = m_eventtype->GetCurrentSelection();
  e.channel = m_channel->GetSelection() + 1;
  e.key = m_key->GetValue();
  e.low_value = m_LowValue->GetValue();
  e.high_value = m_HighValue->GetValue();
  e.start = m_StartValue->GetValue();
  e.length = m_LengthValue->GetValue();
  e.useNoteOff = m_noteOff->GetValue();
}

void GOMidiEventSendTab::OnNewClick(wxCommandEvent &event) {
  StoreEvent();
  m_current = m_midi.AddNewEvent();
  LoadEvent();
}

void GOMidiEventSendTab::OnDeleteClick(wxCommandEvent &event) {
  m_midi.DeleteEvent(m_current);
  m_current = 0;
  LoadEvent();
}

void GOMidiEventSendTab::OnEventChange(wxCommandEvent &event) {
  StoreEvent();
  m_current = m_eventno->GetSelection();
  LoadEvent();
}

void GOMidiEventSendTab::OnCopyClick(wxCommandEvent &event) {
  m_midi.GetEvent(m_current) = CopyEvent();
  LoadEvent();
}

GOMidiSenderEventPattern GOMidiEventSendTab::CopyEvent() {
  GOMidiReceiverEventPattern recv = m_recv->GetCurrentEvent();
  GOMidiSenderEventPattern e;

  // try to fill e.deviceId as the id of the bound output device of the input
  // device
  const GOMidiDeviceConfig *pInDev = recv.deviceId
    ? m_MidiIn.FindByLogicalName(
      m_MidiMap.GetDeviceLogicalNameById(recv.deviceId))
    : NULL;
  const GOMidiDeviceConfig *pOutDev = pInDev ? pInDev->p_OutputDevice : NULL;

  e.deviceId
    = pOutDev ? m_MidiMap.GetDeviceIdByLogicalName(pOutDev->m_LogicalName) : 0;

  e.type = MIDI_S_NONE;
  e.channel = 1;
  e.key = 1;
  e.low_value = 0;
  e.high_value = 127;

  switch (m_midi.GetType()) {
  case MIDI_SEND_MANUAL:
    if (
      recv.type == MIDI_M_NOTE || recv.type == MIDI_M_NOTE_NO_VELOCITY
      || recv.type == MIDI_M_NOTE_SHORT_OCTAVE
      || recv.type == MIDI_M_NOTE_NORMAL) {
      e.type = recv.type == MIDI_M_NOTE_NO_VELOCITY ? MIDI_S_NOTE_NO_VELOCITY
                                                    : MIDI_S_NOTE;
      e.channel = recv.channel;
      e.low_value = recv.low_value ? recv.low_value - 1 : 0;
      e.high_value = recv.high_value;
    }
    break;
  case MIDI_SEND_ENCLOSURE:
    e.channel = recv.channel;
    e.key = recv.key;
    e.low_value = recv.low_value;
    e.high_value = recv.high_value;

    if (recv.type == MIDI_M_CTRL_CHANGE)
      e.type = MIDI_S_CTRL;
    else if (recv.type == MIDI_M_NRPN)
      e.type = MIDI_S_NRPN;
    else if (recv.type == MIDI_M_RPN)
      e.type = MIDI_S_RPN;
    break;
  default:
    e.channel = recv.channel;
    e.key = recv.key;
    e.low_value = recv.low_value;

    if (recv.type == MIDI_M_NOTE)
      e.type = MIDI_S_NOTE;
    else if (recv.type == MIDI_M_CTRL_CHANGE)
      e.type = MIDI_S_CTRL;
    else if (recv.type == MIDI_M_NRPN)
      e.type = MIDI_S_NRPN;
    else if (recv.type == MIDI_M_RPN)
      e.type = MIDI_S_RPN;
    else if (recv.type == MIDI_M_SYSEX_RODGERS_STOP_CHANGE)
      e.type = MIDI_S_RODGERS_STOP_CHANGE;
    else if (recv.type == MIDI_M_RPN_RANGE) {
      e.type = MIDI_S_RPN_RANGE;
      e.high_value = recv.high_value;
    } else if (recv.type == MIDI_M_NRPN_RANGE) {
      e.type = MIDI_S_NRPN_RANGE;
      e.high_value = recv.high_value;
    } else if (recv.type == MIDI_M_PGM_RANGE) {
      e.type = MIDI_S_PGM_RANGE;
      e.high_value = recv.high_value;
    }
  }
  if (e.channel <= 0)
    e.channel = 1;

  return e;
}
