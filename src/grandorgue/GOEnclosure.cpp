/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEnclosure.h"

#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GODocument.h"
#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

GOEnclosure::GOEnclosure(GODefinitionFile *organfile)
  : m_midi(organfile, MIDI_RECV_ENCLOSURE),
    m_sender(organfile, MIDI_SEND_ENCLOSURE),
    m_shortcut(organfile, KEY_RECV_ENCLOSURE),
    m_organfile(organfile),
    m_AmpMinimumLevel(0),
    m_MIDIInputNumber(0),
    m_MIDIValue(0),
    m_Name(),
    m_Displayed1(false),
    m_Displayed2(false) {
  m_organfile->RegisterEventHandler(this);
  m_organfile->RegisterMidiConfigurator(this);
  m_organfile->RegisterPlaybackStateHandler(this);
}

GOEnclosure::~GOEnclosure() {}

void GOEnclosure::Init(
  GOConfigReader &cfg, wxString group, wxString Name, unsigned def_value) {
  m_organfile->RegisterSaveableObject(this);
  m_group = group;
  m_Name = Name;
  Set(cfg.ReadInteger(
    CMBSetting, m_group, wxT("Value"), 0, 127, false, def_value));
  m_midi.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_shortcut.Load(cfg, m_group);
  m_AmpMinimumLevel = 1;
}

void GOEnclosure::Load(GOConfigReader &cfg, wxString group, int enclosure_nb) {
  m_organfile->RegisterSaveableObject(this);
  m_group = group;
  m_Name = cfg.ReadStringNotEmpty(ODFSetting, m_group, wxT("Name"));
  m_Displayed1
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, true);
  m_Displayed2
    = cfg.ReadBoolean(ODFSetting, m_group, wxT("Displayed"), false, false);
  m_AmpMinimumLevel
    = cfg.ReadInteger(ODFSetting, m_group, wxT("AmpMinimumLevel"), 0, 100);
  m_MIDIInputNumber = cfg.ReadInteger(
    ODFSetting, m_group, wxT("MIDIInputNumber"), 0, 200, false, 0);
  Set(cfg.ReadInteger(CMBSetting, m_group, wxT("Value"), 0, 127, false, 127));
  m_midi.SetIndex(enclosure_nb);
  m_midi.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_shortcut.Load(cfg, m_group);
}

void GOEnclosure::Save(GOConfigWriter &cfg) {
  m_midi.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
  m_shortcut.Save(cfg, m_group);
  cfg.WriteInteger(m_group, wxT("Value"), m_MIDIValue);
}

void GOEnclosure::Set(int n) {
  if (n < 0)
    n = 0;
  if (n > 127)
    n = 127;
  m_MIDIValue = n;
  m_sender.SetValue(m_MIDIValue);
  m_organfile->UpdateVolume();
  m_organfile->ControlChanged(this);
}

int GOEnclosure::GetMIDIInputNumber() { return m_MIDIInputNumber; }

float GOEnclosure::GetAttenuation() {
  static const float scale = 1.0 / 12700.0;
  return (float)(m_MIDIValue * (100 - m_AmpMinimumLevel) + 127 * m_AmpMinimumLevel)
    * scale;
}

void GOEnclosure::Scroll(bool scroll_up) {
  Set(m_MIDIValue + (scroll_up ? 3 : -3));
}

void GOEnclosure::ProcessMidi(const GOMidiEvent &event) {
  int value;
  if (m_midi.Match(event, value) == MIDI_MATCH_CHANGE)
    Set(value);
}

void GOEnclosure::HandleKey(int key) {
  switch (m_shortcut.Match(key)) {
  case KEY_MATCH:
    Set(m_MIDIValue + 8);
    break;

  case KEY_MATCH_MINUS:
    Set(m_MIDIValue - 8);
    break;
  default:
    break;
  }
}

const wxString &GOEnclosure::GetName() { return m_Name; }

int GOEnclosure::GetValue() { return m_MIDIValue; }

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

void GOEnclosure::StartPlayback() {}

void GOEnclosure::PrepareRecording() { m_sender.SetValue(m_MIDIValue); }

void GOEnclosure::SetElementID(int id) {
  m_midi.SetElementID(id);
  m_sender.SetElementID(id);
}

wxString GOEnclosure::GetMidiType() { return _("Enclosure"); }

wxString GOEnclosure::GetMidiName() { return GetName(); }

void GOEnclosure::ShowConfigDialog() {
  wxString title = wxString::Format(
    _("Midi-Settings for %s - %s"),
    GetMidiType().c_str(),
    GetMidiName().c_str());

  m_organfile->GetDocument()->ShowMIDIEventDialog(
    this, title, &m_midi, &m_sender, &m_shortcut);
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
