/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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
  GOOrganModel &organModel,
  GOMidiReceiverType midi_type,
  bool pushbutton,
  bool isPiston)
  : GOMidiConfigurator(organModel),
    r_MidiMap(organModel.GetConfig().GetMidiMap()),
    r_OrganModel(organModel),
    m_midi(organModel, midi_type),
    m_sender(organModel, MIDI_SEND_BUTTON),
    m_shortcut(KEY_RECV_BUTTON),
    m_Pushbutton(pushbutton),
    m_Displayed(false),
    m_Name(),
    m_Engaged(false),
    m_DisplayInInvertedState(false),
    m_ReadOnly(false),
    m_IsPiston(isPiston) {
  organModel.RegisterEventHandler(this);
  organModel.RegisterMidiConfigurator(this);
  organModel.RegisterSoundStateHandler(this);
}

void GOButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_Name = name;
  m_Displayed = false;
  m_DisplayInInvertedState = false;
  if (!m_ReadOnly) {
    m_midi.Load(cfg, group, r_MidiMap);
    m_shortcut.Load(cfg, group);
  }
  m_sender.Load(cfg, group, r_MidiMap);
}

void GOButtonControl::Load(GOConfigReader &cfg, const wxString &group) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_Name = cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), true, m_Name);
  m_Displayed
    = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
  m_DisplayInInvertedState = cfg.ReadBoolean(
    ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
  if (!m_ReadOnly) {
    m_midi.Load(cfg, group, r_MidiMap);
    m_shortcut.Load(cfg, group);
  }
  m_sender.Load(cfg, group, r_MidiMap);
}

void GOButtonControl::Save(GOConfigWriter &cfg) {
  if (!m_ReadOnly) {
    m_midi.Save(cfg, m_group, r_MidiMap);
    m_shortcut.Save(cfg, m_group);
  }
  m_sender.Save(cfg, m_group, r_MidiMap);
}

bool GOButtonControl::IsDisplayed() { return m_Displayed; }

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
  r_OrganModel.SendControlChanged(this);
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
