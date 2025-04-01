/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOButtonControl.h"

#include <wx/intl.h>

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
  : GOMidiObjectWithShortcut(
    organModel,
    midiTypeCode,
    midiTypeName,
    MIDI_SEND_BUTTON,
    midiType,
    GOMidiShortcutReceiver::KEY_RECV_BUTTON),
    m_Pushbutton(pushbutton),
    m_Displayed(false),
    m_Engaged(false),
    m_DisplayInInvertedState(false),
    m_ReadOnly(false),
    m_IsPiston(isPiston) {}

void GOButtonControl::Init(
  GOConfigReader &cfg, const wxString &group, const wxString &name) {
  GOMidiObjectWithShortcut::Init(cfg, group, name);
  m_Displayed = false;
  m_DisplayInInvertedState = false;
}

void GOButtonControl::Load(GOConfigReader &cfg, const wxString &group) {
  GOMidiObjectWithShortcut::Load(
    cfg,
    group,
    cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), true),
    false);
  m_Displayed
    = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
  m_DisplayInInvertedState = cfg.ReadBoolean(
    ODFSetting, group, wxT("DisplayInInvertedState"), false, false);
}

void GOButtonControl::OnShortcutKeyReceived(
  GOMidiShortcutReceiver::MatchType matchType, int key) {
  switch (matchType) {
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
  m_Engaged = onoff;
  SendCurrentMidiValue();
  r_OrganModel.SendControlChanged(this);
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