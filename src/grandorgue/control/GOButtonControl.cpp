/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "control/GOButtonControl.h"

#include "GODocument.h"
#include "GOOrganController.h"

GOButtonControl::GOButtonControl(
  GOOrganController *organController,
  GOMidiReceiverType midi_type,
  bool pushbutton,
  bool isPiston)
  : m_OrganController(organController),
    m_midi(organController, midi_type),
    m_sender(organController, MIDI_SEND_BUTTON),
    m_shortcut(organController, KEY_RECV_BUTTON),
    m_Pushbutton(pushbutton),
    m_Displayed(false),
    m_Name(),
    m_Engaged(false),
    m_DisplayInInvertedState(false),
    m_ReadOnly(false),
    m_IsPiston(isPiston) {
  m_OrganController->RegisterEventHandler(this);
  m_OrganController->RegisterMidiConfigurator(this);
  m_OrganController->RegisterPlaybackStateHandler(this);
}

GOButtonControl::~GOButtonControl() {}

void GOButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;
  m_Name = name;
  m_Displayed = false;
  m_DisplayInInvertedState = false;
  if (!m_ReadOnly) {
    m_midi.Load(cfg, group, m_OrganController->GetSettings().GetMidiMap());
    m_shortcut.Load(cfg, group);
  }
  m_sender.Load(cfg, group, m_OrganController->GetSettings().GetMidiMap());
}

void GOButtonControl::Load(GOConfigReader &cfg, const wxString &group) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;
  m_Name = cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), true, m_Name);
  m_Displayed
    = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
  m_DisplayInInvertedState = cfg.ReadBoolean(
    ODFSetting, group, wxT("DisplayInInvertedState"), false, false);

  GOMidiMap &midiMap = m_OrganController->GetSettings().GetMidiMap();

  if (!m_ReadOnly) {
    m_midi.Load(cfg, group, midiMap);
    m_shortcut.Load(cfg, group);
  }
  m_sender.Load(cfg, group, midiMap);
}

void GOButtonControl::Save(GOConfigWriter &cfg) {
  if (!m_ReadOnly) {
    m_midi.Save(cfg, m_group, m_OrganController->GetSettings().GetMidiMap());
    m_shortcut.Save(cfg, m_group);
  }
  m_sender.Save(cfg, m_group, m_OrganController->GetSettings().GetMidiMap());
}

bool GOButtonControl::IsDisplayed() { return m_Displayed; }

bool GOButtonControl::IsReadOnly() { return m_ReadOnly; }

const wxString &GOButtonControl::GetName() { return m_Name; }

void GOButtonControl::HandleKey(int key) {
  if (m_ReadOnly)
    return;
  switch (m_shortcut.Match(key)) {
  case KEY_MATCH:
    Push();
    break;

  default:
    break;
  }
}

void GOButtonControl::Push() {
  if (m_ReadOnly)
    return;
  Set(m_Engaged ^ true);
}

void GOButtonControl::Set(bool on) {}

void GOButtonControl::AbortPlayback() {
  m_sender.SetDisplay(false);
  m_sender.SetName(wxEmptyString);
}

void GOButtonControl::PreparePlayback() {
  m_midi.PreparePlayback();
  m_sender.SetName(m_Name);
}

void GOButtonControl::StartPlayback() {}

void GOButtonControl::PrepareRecording() { m_sender.SetDisplay(m_Engaged); }

void GOButtonControl::ProcessMidi(const GOMidiEvent &event) {
  if (m_ReadOnly)
    return;
  switch (m_midi.Match(event)) {
  case MIDI_MATCH_CHANGE:
    Push();
    break;

  case MIDI_MATCH_ON:
    if (m_Pushbutton)
      Push();
    else
      Set(true);
    break;

  case MIDI_MATCH_OFF:
    if (!m_Pushbutton)
      Set(false);
    break;

  default:
    break;
  }
}

void GOButtonControl::Display(bool onoff) {
  if (m_Engaged == onoff)
    return;
  m_sender.SetDisplay(onoff);
  m_Engaged = onoff;
  m_OrganController->ControlChanged(this);
}

bool GOButtonControl::IsEngaged() const { return m_Engaged; }

bool GOButtonControl::DisplayInverted() const {
  return m_DisplayInInvertedState;
}

void GOButtonControl::SetElementID(int id) {
  if (!m_ReadOnly) {
    m_midi.SetElementID(id);
    m_sender.SetElementID(id);
  }
}

void GOButtonControl::SetShortcutKey(unsigned key) {
  m_shortcut.SetShortcut(key);
}

void GOButtonControl::SetPreconfigIndex(unsigned index) {
  m_midi.SetIndex(index);
}

wxString GOButtonControl::GetMidiName() { return GetName(); }

void GOButtonControl::ShowConfigDialog() {
  wxString title = wxString::Format(
    _("Midi-Settings for %s - %s"),
    GetMidiType().c_str(),
    GetMidiName().c_str());

  GOMidiReceiver *midi = &m_midi;
  GOKeyReceiver *key = &m_shortcut;
  if (IsReadOnly()) {
    midi = NULL;
    key = NULL;
  }

  m_OrganController->ShowMIDIEventDialog(this, title, midi, &m_sender, key);
}

wxString GOButtonControl::GetElementStatus() {
  return m_Engaged ? _("ON") : _("OFF");
}

std::vector<wxString> GOButtonControl::GetElementActions() {
  std::vector<wxString> actions;
  actions.push_back(_("trigger"));
  return actions;
}

void GOButtonControl::TriggerElementActions(unsigned no) {
  if (no == 0)
    Push();
}
