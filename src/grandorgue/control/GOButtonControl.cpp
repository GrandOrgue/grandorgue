/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOButtonControl.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "model/GOOrganModel.h"

#include "GODocument.h"

GOButtonControl::GOButtonControl(
  GOOrganModel &organModel,
  const wxString &midiTypeCode,
  const wxString &midiTypeName,
  GOMidiReceiverType midiType,
  bool pushbutton,
  bool isPiston)
  : GOMidiReceivingSendingObject(
    organModel,
    midiTypeCode,
    midiTypeName,
    MIDI_SEND_BUTTON,
    midiType,
    &m_shortcut),
    m_shortcut(GOMidiShortcutReceiver::KEY_RECV_BUTTON),
    m_Pushbutton(pushbutton),
    m_Displayed(false),
    m_Engaged(false),
    m_DisplayInInvertedState(false),
    m_ReadOnly(false),
    m_IsPiston(isPiston) {}

void GOButtonControl::LoadMidiObject(
  GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiReceivingSendingObject::LoadMidiObject(cfg, group, midiMap);
  if (!m_ReadOnly) {
    m_shortcut.Load(cfg, group);
  }
}

void GOButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  GOMidiReceivingSendingObject::Init(cfg, group, name);
  m_Displayed = false;
  m_DisplayInInvertedState = false;
}

void GOButtonControl::Load(GOConfigReader &cfg, const wxString &group) {
  GOMidiReceivingSendingObject::Load(
    cfg, group, cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), true));
  m_Displayed
    = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
  m_DisplayInInvertedState = cfg.ReadBoolean(
    ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
}

void GOButtonControl::SaveMidiObject(
  GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) {
  GOMidiReceivingSendingObject::SaveMidiObject(cfg, group, midiMap);
  if (!m_ReadOnly) {
    m_shortcut.Save(cfg, group);
  }
}

bool GOButtonControl::IsDisplayed() { return m_Displayed; }

void GOButtonControl::HandleKey(int key) {
  if (m_ReadOnly)
    return;
  switch (m_shortcut.Match(key)) {
  case GOMidiShortcutReceiver::KEY_MATCH:
    Push();
    break;

  default:
    break;
  }
}

void GOButtonControl::Push() {
  if (m_ReadOnly)
    return;
  SetButtonState(m_Engaged ^ true);
}

void GOButtonControl::SetButtonState(bool on) {}

void GOButtonControl::PrepareRecording() {
  GOMidiReceivingSendingObject::PrepareRecording();
  SendMidiValue(m_Engaged);
}

void GOButtonControl::AbortPlayback() {
  SendMidiValue(false);
  GOMidiReceivingSendingObject::AbortPlayback();
}

void GOButtonControl::OnMidiReceived(
  const GOMidiEvent &event, GOMidiMatchType matchType, int key, int value) {
  switch (matchType) {
  case MIDI_MATCH_CHANGE:
    Push();
    break;

  case MIDI_MATCH_ON:
    if (m_Pushbutton)
      Push();
    else
      SetButtonState(true);
    break;

  case MIDI_MATCH_OFF:
    if (!m_Pushbutton)
      SetButtonState(false);
    break;

  default:
    break;
  }
}

void GOButtonControl::Display(bool onoff) {
  if (m_Engaged == onoff)
    return;
  SendMidiValue(onoff);
  m_Engaged = onoff;
  r_OrganModel.SendControlChanged(this);
}

bool GOButtonControl::IsEngaged() const { return m_Engaged; }

bool GOButtonControl::DisplayInverted() const {
  return m_DisplayInInvertedState;
}

void GOButtonControl::SetShortcutKey(unsigned key) {
  m_shortcut.SetShortcut(key);
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
