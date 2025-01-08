/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEnclosure.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

#include "GOOrganModel.h"

GOEnclosure::GOEnclosure(GOOrganModel &organModel)
  : GOMidiConfigurator(organModel),
    r_OrganModel(organModel),
    r_MidiMap(organModel.GetConfig().GetMidiMap()),
    m_midi(organModel, MIDI_RECV_ENCLOSURE),
    m_sender(organModel, MIDI_SEND_ENCLOSURE),
    m_shortcut(KEY_RECV_ENCLOSURE),
    m_Name(),
    m_DefaultAmpMinimumLevel(0),
    m_MIDIInputNumber(0),
    m_Displayed1(false),
    m_Displayed2(false),
    m_AmpMinimumLevel(0),
    m_MIDIValue(0) {
  organModel.RegisterEventHandler(this);
  organModel.RegisterMidiConfigurator(this);
  organModel.RegisterSoundStateHandler(this);
}

void GOEnclosure::Init(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &name,
  uint8_t defaultValue) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_Name = name;
  m_DefaultAmpMinimumLevel = 0;
  LoadFromCmb(cfg, defaultValue);
}

static const wxString WX_AMP_MINIMUM_LEVEL = wxT("AmpMinimumLevel");
static const wxString WX_VALUE = wxT("Value");

void GOEnclosure::LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue) {
  m_midi.Load(cfg, m_group, r_MidiMap);
  m_sender.Load(cfg, m_group, r_MidiMap);
  m_shortcut.Load(cfg, m_group);
  m_AmpMinimumLevel = cfg.ReadInteger(
    CMBSetting,
    m_group,
    WX_AMP_MINIMUM_LEVEL,
    0,
    100,
    false,
    m_DefaultAmpMinimumLevel);
  SetMidiValue(cfg.ReadInteger(
    CMBSetting, m_group, WX_VALUE, 0, MAX_MIDI_VALUE, false, defaultValue));
}

void GOEnclosure::Load(
  GOConfigReader &cfg, const wxString &group, int enclosure_nb) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_Name = cfg.ReadStringNotEmpty(ODFSetting, m_group, wxT("Name"));
  m_Displayed1
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, true);
  m_Displayed2
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, false);
  m_DefaultAmpMinimumLevel
    = cfg.ReadInteger(ODFSetting, m_group, WX_AMP_MINIMUM_LEVEL, 0, 100);
  m_MIDIInputNumber = cfg.ReadInteger(
    ODFSetting, m_group, wxT("MIDIInputNumber"), 0, 200, false, 0);
  m_midi.SetIndex(enclosure_nb);
  LoadFromCmb(cfg, MAX_MIDI_VALUE);
}

void GOEnclosure::Save(GOConfigWriter &cfg) {
  m_midi.Save(cfg, m_group, r_MidiMap);
  m_sender.Save(cfg, m_group, r_MidiMap);
  m_shortcut.Save(cfg, m_group);
  cfg.WriteInteger(m_group, WX_AMP_MINIMUM_LEVEL, m_AmpMinimumLevel);
  cfg.WriteInteger(m_group, WX_VALUE, m_MIDIValue);
}

void GOEnclosure::SetMidiValue(uint8_t n) {
  if (n != m_MIDIValue) {
    m_MIDIValue = n;
    m_sender.SetValue(n);
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
  SetIntMidiValue(m_MIDIValue + (scroll_up ? 4 : -4));
}

void GOEnclosure::ProcessMidi(const GOMidiEvent &event) {
  int value;

  if (m_midi.Match(event, value) == MIDI_MATCH_CHANGE)
    SetIntMidiValue(value);
}

void GOEnclosure::HandleKey(int key) {
  switch (m_shortcut.Match(key)) {
  case KEY_MATCH:
    SetIntMidiValue(m_MIDIValue + 8);
    break;

  case KEY_MATCH_MINUS:
    SetIntMidiValue(m_MIDIValue - 8);
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

void GOEnclosure::AbortPlayback() {
  m_sender.SetValue(0);
  m_sender.SetName(wxEmptyString);
}

void GOEnclosure::PreparePlayback() {
  m_midi.PreparePlayback();
  m_sender.SetName(m_Name);
}

void GOEnclosure::PrepareRecording() { m_sender.SetValue(m_MIDIValue); }

void GOEnclosure::SetElementID(int id) {
  m_midi.SetElementID(id);
  m_sender.SetElementID(id);
}

const wxString WX_MIDI_TYPE_CODE = wxT("Enclosure");
const wxString WX_MIDI_TYPE = _("Enclosure");

const wxString &GOEnclosure::GetMidiTypeCode() const {
  return WX_MIDI_TYPE_CODE;
}

const wxString &GOEnclosure::GetMidiType() const { return WX_MIDI_TYPE; }

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
