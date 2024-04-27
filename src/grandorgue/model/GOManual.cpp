/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOManual.h"

#include <wx/intl.h>

#include "combinations/control/GODivisionalButtonControl.h"
#include "config/GOConfig.h"
#include "config/GOConfigReader.h"

#include "GOCoupler.h"
#include "GODocument.h"
#include "GOOrganModel.h"
#include "GOStop.h"
#include "GOSwitch.h"
#include "GOTremulant.h"

GOManual::GOManual(GOOrganModel &organModel)
  : GOMidiConfigurator(organModel),
    r_OrganModel(organModel),
    r_MidiMap(organModel.GetConfig().GetMidiMap()),
    m_group(wxT("---")),
    m_midi(organModel, MIDI_RECV_MANUAL),
    m_sender(organModel, MIDI_SEND_MANUAL),
    m_division(organModel, MIDI_SEND_MANUAL),
    m_InputCouplers(),
    m_KeyVelocity(0),
    m_RemoteVelocity(),
    m_Velocity(),
    m_DivisionState(),
    m_Velocities(),
    m_manual_number(0),
    m_first_accessible_logical_key_nb(0),
    m_nb_logical_keys(0),
    m_first_accessible_key_midi_note_nb(0),
    m_nb_accessible_keys(0),
    m_UnisonOff(0),
    m_MIDIInputNumber(0),
    m_tremulant_ids(0),
    m_GlobalSwitchIds(0),
    m_name(),
    m_stops(0),
    m_couplers(0),
    m_divisionals(0),
    m_ODFCouplerCount(0),
    m_displayed(false),
    m_DivisionalTemplate(organModel) {
  m_InputCouplers.push_back(NULL);
  organModel.RegisterCombinationButtonSet(this);
  organModel.RegisterEventHandler(this);
  organModel.RegisterMidiConfigurator(this);
  organModel.RegisterSoundStateHandler(this);
}

unsigned GOManual::RegisterCoupler(GOCoupler *coupler) {
  m_InputCouplers.push_back(coupler);
  Resize();
  return m_InputCouplers.size() - 1;
}

void GOManual::Resize() {
  m_Velocity.resize(m_nb_logical_keys);
  m_DivisionState.resize(m_nb_logical_keys);
  m_RemoteVelocity.resize(m_nb_logical_keys);
  m_Velocities.resize(m_nb_logical_keys);
  for (unsigned i = 0; i < m_Velocities.size(); i++)
    m_Velocities[i].resize(m_InputCouplers.size());
}

void GOManual::Init(
  GOConfigReader &cfg,
  wxString group,
  int manualNumber,
  unsigned first_midi,
  unsigned keys) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_name = wxString::Format(
    _("Coupling manual %d"),
    manualNumber - r_OrganModel.GetODFManualCount() + 1);
  m_nb_logical_keys = keys;
  m_first_accessible_logical_key_nb = 1;
  m_first_accessible_key_midi_note_nb = first_midi;
  m_nb_accessible_keys = keys;
  m_MIDIInputNumber = 0;
  m_displayed = false;
  m_manual_number = manualNumber;
  for (unsigned i = 0; i < 128; i++)
    m_MidiMap[i] = i;

  m_stops.resize(0);
  m_couplers.resize(0);
  m_ODFCouplerCount = 0;
  m_tremulant_ids.resize(0);
  m_GlobalSwitchIds.resize(0);
  m_divisionals.resize(0);
  m_midi.Load(cfg, group, r_MidiMap);
  m_sender.Load(cfg, group, r_MidiMap);
  m_division.Load(cfg, group + wxT("Division"), r_MidiMap);

  SetElementID(r_OrganModel.GetRecorderElementID(
    wxString::Format(wxT("M%d"), m_manual_number)));

  Resize();
  m_KeyVelocity.resize(m_nb_accessible_keys);
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
}

void GOManual::Load(
  GOConfigReader &cfg, const wxString &group, int manualNumber) {
  r_OrganModel.RegisterSaveableObject(this);
  m_group = group;
  m_name = cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"));
  m_nb_logical_keys
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLogicalKeys"), 1, 192);
  m_first_accessible_logical_key_nb = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("FirstAccessibleKeyLogicalKeyNumber"),
    1,
    m_nb_logical_keys);
  m_first_accessible_key_midi_note_nb = cfg.ReadInteger(
    ODFSetting, group, wxT("FirstAccessibleKeyMIDINoteNumber"), 0, 127);
  m_nb_accessible_keys
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfAccessibleKeys"), 0, 85);
  m_MIDIInputNumber = cfg.ReadInteger(
    ODFSetting, group, wxT("MIDIInputNumber"), 0, 200, false, 0);
  m_displayed
    = cfg.ReadBoolean(ODFSetting, group, wxT("Displayed"), false, false);
  unsigned nb_stops
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfStops"), 0, 999);
  m_ODFCouplerCount = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfCouplers"), 0, 999, false);
  unsigned nb_tremulants = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("NumberOfTremulants"),
    0,
    r_OrganModel.GetTremulantCount(),
    false);
  unsigned nb_switches = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("NumberOfSwitches"),
    0,
    r_OrganModel.GetSwitchCount(),
    false);
  m_manual_number = manualNumber;

  m_midi.SetIndex(manualNumber);

  for (unsigned i = 0; i < 128; i++)
    m_MidiMap[i] = cfg.ReadInteger(
      ODFSetting,
      group,
      wxString::Format(wxT("MIDIKey%03d"), i),
      0,
      127,
      false,
      i);

  wxString buffer;

  m_stops.resize(0);
  for (unsigned i = 0; i < nb_stops; i++) {
    m_stops.push_back(
      new GOStop(r_OrganModel, GetFirstLogicalKeyMIDINoteNumber()));
    buffer.Printf(wxT("Stop%03d"), i + 1);
    buffer.Printf(
      wxT("Stop%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 999));
    cfg.MarkGroupInUse(buffer);
    m_stops[i]->Load(cfg, buffer);
    m_stops[i]->SetElementID(r_OrganModel.GetRecorderElementID(
      wxString::Format(wxT("M%dS%d"), m_manual_number, i)));
  }

  m_couplers.resize(0);
  for (unsigned i = 0; i < m_ODFCouplerCount; i++) {
    m_couplers.push_back(new GOCoupler(r_OrganModel, m_manual_number));
    buffer.Printf(wxT("Coupler%03d"), i + 1);
    buffer.Printf(
      wxT("Coupler%03d"), cfg.ReadInteger(ODFSetting, group, buffer, 1, 999));
    cfg.MarkGroupInUse(buffer);
    m_couplers[i]->Load(cfg, buffer);
    m_couplers[i]->SetElementID(r_OrganModel.GetRecorderElementID(
      wxString::Format(wxT("M%dC%d"), m_manual_number, i)));
  }

  m_tremulant_ids.resize(0);
  for (unsigned i = 0; i < nb_tremulants; i++) {
    buffer.Printf(wxT("Tremulant%03d"), i + 1);
    unsigned new_id = cfg.ReadInteger(
      ODFSetting, group, buffer, 1, r_OrganModel.GetTremulantCount());
    for (unsigned j = 0; j < m_tremulant_ids.size(); j++)
      if (m_tremulant_ids[j] == new_id)
        throw wxString::Format(
          _("Manual %d: Tremulant%03d already in use"),
          m_manual_number,
          new_id);
    m_tremulant_ids.push_back(new_id);
  }

  m_GlobalSwitchIds.resize(0);
  for (unsigned i = 0; i < nb_switches; i++) {
    buffer.Printf(wxT("Switch%03d"), i + 1);
    unsigned globalSwitchId = cfg.ReadInteger(
      ODFSetting, group, buffer, 1, r_OrganModel.GetSwitchCount());
    for (unsigned j = 0; j < m_GlobalSwitchIds.size(); j++)
      if (m_GlobalSwitchIds[j] == globalSwitchId)
        throw wxString::Format(
          _("Manual %d: Switch%03d already in use"),
          m_manual_number,
          globalSwitchId);
    m_GlobalSwitchIds.push_back(globalSwitchId);
    r_OrganModel.GetSwitch(globalSwitchId - 1)
      ->AssociateWithManual(m_manual_number, i);
  }

  m_midi.Load(cfg, group, r_MidiMap);
  m_sender.Load(cfg, group, r_MidiMap);
  m_division.Load(cfg, group + wxT("Division"), r_MidiMap);

  SetElementID(r_OrganModel.GetRecorderElementID(
    wxString::Format(wxT("M%d"), m_manual_number)));

  Resize();
  m_KeyVelocity.resize(m_nb_accessible_keys);
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
}

void GOManual::LoadDivisionals(GOConfigReader &cfg) {
  unsigned nDivisionals = cfg.ReadInteger(
    ODFSetting, m_group, wxT("NumberOfDivisionals"), 0, 999, false);
  wxString buffer;

  m_DivisionalTemplate.InitDivisional(*this);
  m_divisionals.resize(0);
  for (unsigned i = 0; i < nDivisionals; i++) {
    m_divisionals.push_back(
      new GODivisionalButtonControl(r_OrganModel, m_manual_number, i));

    buffer.Printf(wxT("Divisional%03d"), i + 1);
    buffer.Printf(
      wxT("Divisional%03d"),
      cfg.ReadInteger(ODFSetting, m_group, buffer, 1, 999));
    cfg.MarkGroupInUse(buffer);
    m_divisionals[i]->Load(cfg, buffer);
  }
}

void GOManual::SetOutput(unsigned note, unsigned velocity) {
  if (note < 0 || note >= m_DivisionState.size())
    return;
  velocity >>= 2;

  if (m_DivisionState[note] == velocity)
    return;
  m_DivisionState[note] = velocity;

  for (unsigned i = 0; i < m_stops.size(); i++)
    m_stops[i]->SetKey(note + 1, velocity);

  int midi_note = note + m_first_accessible_key_midi_note_nb
    - m_first_accessible_logical_key_nb + 1;
  if (midi_note >= 0 && midi_note < 127)
    m_division.SetKey(midi_note, velocity);
}

void GOManual::PropagateKeyToCouplers(unsigned note) {
  if (note < m_Velocity.size()) {
    auto &noteVelocities = m_Velocities[note];

    for (auto pCoupler : m_couplers)
      pCoupler->SetKey(note, noteVelocities, m_InputCouplers);
  }
}

void GOManual::SetKey(unsigned note, unsigned velocity, unsigned couplerID) {
  if (note >= m_Velocity.size())
    return;

  if (m_Velocities[note][couplerID] == velocity)
    return;

  m_Velocities[note][couplerID] = velocity;
  m_Velocity[note] = m_Velocities[note][0];
  m_RemoteVelocity[note] = 0;
  for (unsigned i = 1; i < m_Velocities[note].size(); i++) {
    if (m_Velocity[note] < m_Velocities[note][i])
      m_Velocity[note] = m_Velocities[note][i];
    if (m_RemoteVelocity[note] < m_Velocities[note][i])
      m_RemoteVelocity[note] = m_Velocities[note][i];
  }

  PropagateKeyToCouplers(note);
  SetOutput(note, m_UnisonOff > 0 ? m_RemoteVelocity[note] : m_Velocity[note]);

  if (
    m_first_accessible_logical_key_nb <= note + 1
    && note <= m_first_accessible_logical_key_nb + m_nb_accessible_keys)
    r_OrganModel.SendControlChanged(this);
}

void GOManual::Set(unsigned note, unsigned velocity) {
  if (
    note < m_first_accessible_key_midi_note_nb
    || note >= m_first_accessible_key_midi_note_nb + m_KeyVelocity.size())
    return;
  if (m_KeyVelocity[note - m_first_accessible_key_midi_note_nb] == velocity)
    return;
  m_KeyVelocity[note - m_first_accessible_key_midi_note_nb] = velocity;
  m_sender.SetKey(note, velocity);
  if (velocity)
    velocity = (velocity << 2) + 3;
  SetKey(
    note - m_first_accessible_key_midi_note_nb
      + m_first_accessible_logical_key_nb - 1,
    velocity,
    0);
}

void GOManual::SetUnisonOff(bool on) {
  if (on) {
    if (m_UnisonOff++)
      return;
  } else {
    if (--m_UnisonOff)
      return;
  }
  for (unsigned note = 0; note < m_Velocity.size(); note++)
    SetOutput(note, on ? m_RemoteVelocity[note] : m_Velocity[note]);
}

GOManual::~GOManual(void) {}

int GOManual::GetMIDIInputNumber() { return m_MIDIInputNumber; }

unsigned GOManual::GetLogicalKeyCount() { return m_nb_logical_keys; }

unsigned GOManual::GetNumberOfAccessibleKeys() { return m_nb_accessible_keys; }

/* TODO: I suspect this could be made private or into something better... */
unsigned GOManual::GetFirstAccessibleKeyMIDINoteNumber() {
  return m_first_accessible_key_midi_note_nb;
}

int GOManual::GetFirstLogicalKeyMIDINoteNumber() {
  return m_first_accessible_key_midi_note_nb - m_first_accessible_logical_key_nb
    + 1;
}

GOStop *GOManual::GetStop(unsigned index) {
  assert(index < m_stops.size());
  return m_stops[index];
}

int GOManual::FindStopByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_stops.size(), i = 0; i < l; i++)
    if (m_stops[i]->GetName() == name) {
      resIndex = i;
      break;
    }
  return resIndex;
}

GOCoupler *GOManual::GetCoupler(unsigned index) {
  assert(index < m_couplers.size());
  return m_couplers[index];
}

int GOManual::FindCouplerByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_couplers.size(), i = 0; i < l; i++)
    if (m_couplers[i]->GetName() == name) {
      resIndex = i;
      break;
    }
  return resIndex;
}

void GOManual::AddCoupler(GOCoupler *coupler) { m_couplers.push_back(coupler); }

GODivisionalButtonControl *GOManual::GetDivisional(unsigned index) {
  assert(index < m_divisionals.size());
  return m_divisionals[index];
}

void GOManual::AddDivisional(GODivisionalButtonControl *divisional) {
  m_divisionals.push_back(divisional);
}

GOCombinationDefinition &GOManual::GetDivisionalTemplate() {
  return m_DivisionalTemplate;
}

GOTremulant *GOManual::GetTremulant(unsigned index) {
  assert(index < m_tremulant_ids.size());
  return r_OrganModel.GetTremulant(m_tremulant_ids[index] - 1);
}

int GOManual::FindTremulantByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_tremulant_ids.size(), i = 0; i < l; i++) {
    int globalIndex = m_tremulant_ids[i] - 1;

    if (
      globalIndex >= 0
      && r_OrganModel.GetTremulant(globalIndex)->GetName() == name) {
      resIndex = globalIndex;
      break;
    }
  }
  return resIndex;
}

GOSwitch *GOManual::GetSwitch(unsigned index) {
  assert(index < m_GlobalSwitchIds.size());
  return r_OrganModel.GetSwitch(m_GlobalSwitchIds[index] - 1);
}

int GOManual::FindSwitchByName(const wxString &name) const {
  int resIndex = -1;

  for (unsigned l = m_GlobalSwitchIds.size(), i = 0; i < l; i++) {
    int globalIndex = m_GlobalSwitchIds[i] - 1;

    if (
      globalIndex >= 0
      && r_OrganModel.GetSwitch(globalIndex)->GetName() == name) {
      resIndex = globalIndex;
      break;
    }
  }
  return resIndex;
}

void GOManual::AllNotesOff() {
  for (unsigned j = 0; j < m_nb_accessible_keys; j++)
    Set(m_first_accessible_key_midi_note_nb + j, 0x00);
}

bool GOManual::IsKeyDown(unsigned midiNoteNumber) {
  if (midiNoteNumber < m_first_accessible_key_midi_note_nb)
    return false;
  if (
    midiNoteNumber
    >= m_first_accessible_key_midi_note_nb + m_nb_accessible_keys)
    return false;
  if (
    midiNoteNumber - m_first_accessible_key_midi_note_nb
      + m_first_accessible_logical_key_nb - 1
    >= m_Velocity.size())
    return false;
  return m_Velocity
           [midiNoteNumber - m_first_accessible_key_midi_note_nb
            + m_first_accessible_logical_key_nb - 1]
    > 0;
}

bool GOManual::IsDisplayed() { return m_displayed; }

void GOManual::Save(GOConfigWriter &cfg) {
  m_midi.Save(cfg, m_group, r_MidiMap);
  m_sender.Save(cfg, m_group, r_MidiMap);
  m_division.Save(cfg, m_group + wxT("Division"), r_MidiMap);
}

void GOManual::AbortPlayback() {
  AllNotesOff();
  m_sender.SetName(wxEmptyString);
}

void GOManual::PreparePlayback() {
  m_midi.PreparePlayback();
  m_KeyVelocity.resize(m_nb_accessible_keys);
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0x00);
  m_division.ResetKey();
  m_UnisonOff = 0;
  for (unsigned i = 0; i < m_Velocity.size(); i++)
    m_Velocity[i] = 0;
  for (unsigned i = 0; i < m_DivisionState.size(); i++)
    m_DivisionState[i] = 0;
  for (unsigned i = 0; i < m_RemoteVelocity.size(); i++)
    m_RemoteVelocity[i] = 0;
  for (unsigned i = 0; i < m_Velocities.size(); i++)
    for (unsigned j = 0; j < m_Velocities[i].size(); j++)
      m_Velocities[i][j] = 0;
  m_sender.SetName(m_name);
}

void GOManual::PrepareRecording() {
  m_sender.ResetKey();
  for (unsigned i = 0; i < m_KeyVelocity.size(); i++)
    if (m_KeyVelocity[i] > 0)
      m_sender.SetKey(
        i + m_first_accessible_key_midi_note_nb, m_KeyVelocity[i]);
}

void GOManual::Update() {
  for (unsigned i = 0; i < m_stops.size(); i++)
    m_stops[i]->Update();

  for (unsigned i = 0; i < m_couplers.size(); i++)
    m_couplers[i]->Update();
}

void GOManual::ProcessMidi(const GOMidiEvent &event) {
  int key, value;

  switch (m_midi.Match(event, m_MidiMap, key, value)) {
  case MIDI_MATCH_ON:
    if (value <= 0)
      value = 1;
    Set(key, value);
    break;

  case MIDI_MATCH_OFF:
    Set(key, 0x00);
    break;

  case MIDI_MATCH_RESET:
    AllNotesOff();
    break;

  default:
    break;
  }
}

void GOManual::HandleKey(int key) {}

void GOManual::Reset() {
  for (unsigned j = 0; j < GetCouplerCount(); j++)
    GetCoupler(j)->Reset();
  for (unsigned j = 0; j < GetDivisionalCount(); j++)
    GetDivisional(j)->Display(false);

  for (unsigned j = 0; j < GetStopCount(); j++)
    GetStop(j)->Reset();
}

void GOManual::SetElementID(int id) {
  m_midi.SetElementID(id);
  m_sender.SetElementID(id);
}

const wxString WX_MIDI_TYPE_CODE = wxT("Manual");
const wxString WX_MIDI_TYPE = _("Manual");

const wxString &GOManual::GetMidiTypeCode() const { return WX_MIDI_TYPE_CODE; }

const wxString &GOManual::GetMidiType() const { return WX_MIDI_TYPE; }

wxString GOManual::GetElementStatus() { return _("-"); }

std::vector<wxString> GOManual::GetElementActions() {
  std::vector<wxString> actions;
  return actions;
}

void GOManual::TriggerElementActions(unsigned no) {
  // Never called
}

void GOManual::UpdateAllButtonsLight(
  GOButtonControl *buttonToLight, int manualIndexOnlyFor) {
  if (manualIndexOnlyFor < 0 || (unsigned)manualIndexOnlyFor == m_manual_number)
    for (GOButtonControl *pDivisional : m_divisionals)
      updateOneButtonLight(pDivisional, buttonToLight);
}
