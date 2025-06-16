/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEnclosure.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrganModel.h"

GOEnclosure::GOEnclosure(GOOrganModel &organModel)
  : GOMidiObjectWithShortcut(
    organModel,
    OBJECT_TYPE_ENCLOSURE,
    MIDI_SEND_ENCLOSURE,
    MIDI_RECV_ENCLOSURE,
    KEY_RECV_ENCLOSURE),
    m_IsOdfDefined(false),
    m_DefaultAmpMinimumLevel(0),
    m_Displayed1(false),
    m_Displayed2(false),
    m_AmpMinimumLevel(0),
    m_MIDIValue(0) {}

static const wxString WX_AMP_MINIMUM_LEVEL = wxT("AmpMinimumLevel");
static const wxString WX_VALUE = wxT("Value");

void GOEnclosure::LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue) {
  m_AmpMinimumLevel = cfg.ReadInteger(
    CMBSetting,
    m_group,
    WX_AMP_MINIMUM_LEVEL,
    0,
    100,
    false,
    m_DefaultAmpMinimumLevel);
  SetEnclosureValue(cfg.ReadInteger(
    CMBSetting, m_group, WX_VALUE, 0, MAX_MIDI_VALUE, false, defaultValue));
}

void GOEnclosure::Init(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &name,
  uint8_t defaultValue) {
  m_IsOdfDefined = false;
  GOMidiReceivingSendingObject::Init(cfg, group, name);
  m_DefaultAmpMinimumLevel = 0;
  LoadFromCmb(cfg, defaultValue);
}

void GOEnclosure::Load(GOConfigReader &cfg, const wxString &group) {
  m_IsOdfDefined = true;
  GOMidiReceivingSendingObject::Load(
    cfg, group, cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name")), true);
  m_Displayed1
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, true);
  m_Displayed2
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, false);
  m_DefaultAmpMinimumLevel
    = cfg.ReadInteger(ODFSetting, m_group, WX_AMP_MINIMUM_LEVEL, 0, 100);
  LoadFromCmb(cfg, MAX_MIDI_VALUE);
}

void GOEnclosure::Save(GOConfigWriter &cfg) {
  GOMidiReceivingSendingObject::Save(cfg);
  cfg.WriteInteger(m_group, WX_AMP_MINIMUM_LEVEL, m_AmpMinimumLevel);
  cfg.WriteInteger(m_group, WX_VALUE, m_MIDIValue);
}

void GOEnclosure::SetEnclosureValue(uint8_t n) {
  if (n != m_MIDIValue) {
    m_MIDIValue = n;
    SendCurrentMidiValue();
  }
  r_OrganModel.UpdateVolume();
  r_OrganModel.SendControlChanged(this);
}

float GOEnclosure::GetAttenuation() {
  static const float scale = 1.0 / 12700.0;
  return (float)(m_MIDIValue * (100 - m_AmpMinimumLevel) + 127 * m_AmpMinimumLevel)
    * scale;
}

void GOEnclosure::Scroll(bool scroll_up) {
  SetIntEnclosureValue(m_MIDIValue + (scroll_up ? 4 : -4));
}

void GOEnclosure::OnMidiReceived(
  const GOMidiEvent &event, GOMidiMatchType matchType, int key, int value) {
  if (matchType == MIDI_MATCH_CHANGE)
    SetEnclosureValue(value);
}

void GOEnclosure::OnShortcutKeyReceived(
  GOMidiShortcutReceiver::MatchType matchType, int key) {
  switch (matchType) {
  case GOMidiShortcutReceiver::KEY_MATCH:
    SetIntEnclosureValue(m_MIDIValue + 8);
    break;

  case GOMidiShortcutReceiver::KEY_MATCH_MINUS:
    SetIntEnclosureValue(m_MIDIValue - 8);
    break;
  default:
    break;
  }
}

bool GOEnclosure::IsDisplayed(bool new_format) {
  if (new_format)
    return m_Displayed2;
  else
    return m_Displayed1;
}

wxString GOEnclosure::GetElementStatus() {
  return wxString::Format(_("%.3f %%"), (m_MIDIValue * 100.0 / 127));
}

std::vector<wxString> GOEnclosure::GetElementActions() {
  std::vector<wxString> actions;
  actions.push_back(_("-"));
  actions.push_back(_("+"));
  return actions;
}

void GOEnclosure::TriggerElementActions(unsigned no) {
  if (no == 0)
    Scroll(false);
  if (no == 1)
    Scroll(true);
}
