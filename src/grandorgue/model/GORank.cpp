/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GORank.h"

#include <algorithm>

#include <wx/intl.h>

#include "GODocument.h"
#include "GODummyPipe.h"
#include "GOOrganController.h"
#include "GOReferencePipe.h"
#include "GOSoundingPipe.h"
#include "GOWindchest.h"
#include "config/GOConfig.h"
#include "config/GOConfigReader.h"

GORank::GORank(GOOrganController *organController)
  : m_OrganController(organController),
    m_Name(),
    m_Pipes(),
    m_StopCount(0),
    m_NoteStopVelocities(),
    m_MaxNoteVelocities(),
    m_FirstMidiNoteNumber(0),
    m_Percussive(false),
    m_WindchestGroup(0),
    m_HarmonicNumber(8),
    m_MinVolume(100),
    m_MaxVolume(100),
    m_RetuneRank(true),
    m_sender(organController, MIDI_SEND_MANUAL),
    m_PipeConfig(NULL, organController, NULL) {
  m_OrganController->RegisterMidiConfigurator(this);
  m_OrganController->RegisterSoundStateHandler(this);
}

GORank::~GORank() {}

void GORank::Resize() {
  m_MaxNoteVelocities.resize(m_Pipes.size());
  m_NoteStopVelocities.resize(m_Pipes.size());
  for (unsigned i = 0; i < m_NoteStopVelocities.size(); i++)
    m_NoteStopVelocities[i].resize(m_StopCount);
}

void GORank::Init(
  GOConfigReader &cfg,
  wxString group,
  wxString name,
  int first_midi_note_number,
  unsigned windchest_nr) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;

  m_FirstMidiNoteNumber = first_midi_note_number;
  m_Name = name;

  m_PipeConfig.Init(cfg, group, wxEmptyString);
  m_WindchestGroup = windchest_nr;
  m_Percussive = false;
  m_HarmonicNumber = 8;
  m_MinVolume = 100;
  m_MaxVolume = 100;
  m_RetuneRank = false;

  GOWindchest *windchest
    = m_OrganController->GetWindchest(m_WindchestGroup - 1);
  windchest->AddRank(this);
  m_PipeConfig.SetParent(&windchest->GetPipeConfig());

  m_Pipes.clear();
  m_sender.Load(
    cfg, group + wxT("Rank"), m_OrganController->GetSettings().GetMidiMap());
  m_PipeConfig.SetName(GetName());
  Resize();
}

void GORank::Load(
  GOConfigReader &cfg, wxString group, int first_midi_note_number) {
  m_OrganController->RegisterSaveableObject(this);
  m_group = group;

  m_FirstMidiNoteNumber = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("FirstMidiNoteNumber"),
    0,
    256,
    first_midi_note_number < 0,
    first_midi_note_number);
  m_Name = cfg.ReadString(ODFSetting, group, wxT("Name"), true);

  unsigned number_of_logical_pipes
    = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfLogicalPipes"), 1, 192);
  m_PipeConfig.Load(cfg, group, wxEmptyString);
  m_WindchestGroup = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("WindchestGroup"),
    1,
    m_OrganController->GetWindchestGroupCount());
  m_Percussive = cfg.ReadBoolean(ODFSetting, group, wxT("Percussive"));
  m_HarmonicNumber = cfg.ReadInteger(
    ODFSetting, group, wxT("HarmonicNumber"), 1, 1024, false, 8);
  m_MinVolume = cfg.ReadFloat(
    ODFSetting, group, wxT("MinVelocityVolume"), 0, 1000, false, 100);
  m_MaxVolume = cfg.ReadFloat(
    ODFSetting, group, wxT("MaxVelocityVolume"), 0, 1000, false, 100);
  m_RetuneRank
    = cfg.ReadBoolean(ODFSetting, group, wxT("AcceptsRetuning"), false, true);

  GOWindchest *windchest
    = m_OrganController->GetWindchest(m_WindchestGroup - 1);
  windchest->AddRank(this);
  m_PipeConfig.SetParent(&windchest->GetPipeConfig());

  m_Pipes.clear();
  for (unsigned i = 0; i < number_of_logical_pipes; i++) {
    wxString buffer;
    buffer.Printf(wxT("Pipe%03u"), i + 1);
    wxString name = cfg.ReadStringTrim(ODFSetting, group, buffer);
    if (name == wxT("DUMMY")) {
      m_Pipes.push_back(
        new GODummyPipe(m_OrganController, this, m_FirstMidiNoteNumber + i));
    } else if (name.StartsWith(wxT("REF:"))) {
      m_Pipes.push_back(new GOReferencePipe(
        m_OrganController, this, m_FirstMidiNoteNumber + i));
    } else {
      m_Pipes.push_back(new GOSoundingPipe(
        m_OrganController,
        this,
        m_Percussive,
        m_WindchestGroup,
        m_FirstMidiNoteNumber + i,
        m_HarmonicNumber,
        m_MinVolume,
        m_MaxVolume,
        m_RetuneRank));
    }
    m_Pipes[i]->Load(cfg, group, buffer);
  }
  m_sender.Load(
    cfg, group + wxT("Rank"), m_OrganController->GetSettings().GetMidiMap());
  m_PipeConfig.SetName(GetName());
  Resize();
}

void GORank::AddPipe(GOPipe *pipe) {
  m_Pipes.push_back(pipe);
  Resize();
}

void GORank::Save(GOConfigWriter &cfg) {
  m_sender.Save(
    cfg, m_group + wxT("Rank"), m_OrganController->GetSettings().GetMidiMap());
}

unsigned GORank::RegisterStop(GOStop *stop) {
  unsigned id = m_StopCount++;
  Resize();
  return id;
}

void GORank::SetKey(int note, unsigned velocity, unsigned stopID) {
  if (note >= 0 && note < (int)m_Pipes.size()) {
    auto &allStopVelocities = m_NoteStopVelocities[note];
    unsigned &thisStopVelocity = allStopVelocities[stopID];
    unsigned oldThisStopVelocity = thisStopVelocity;
    unsigned &maxVelocity = m_MaxNoteVelocities[note];

    thisStopVelocity = velocity;
    if (velocity > maxVelocity || velocity < oldThisStopVelocity) {
      // the max velocity of the pipe is changed
      // find the new max velocity
      maxVelocity = velocity >= maxVelocity
        ? velocity
        : *std::max_element(allStopVelocities.begin(), allStopVelocities.end());
      m_Pipes[note]->Set(maxVelocity);
    }
  }
}

GOPipe *GORank::GetPipe(unsigned index) { return m_Pipes[index]; }

unsigned GORank::GetPipeCount() { return m_Pipes.size(); }

const wxString &GORank::GetName() { return m_Name; }

GOPipeConfigNode &GORank::GetPipeConfig() { return m_PipeConfig; }

void GORank::SetTemperament(const GOTemperament &temperament) {
  for (unsigned j = 0; j < m_Pipes.size(); j++)
    m_Pipes[j]->SetTemperament(temperament);
}

void GORank::AbortPlayback() { m_sender.SetName(wxEmptyString); }

void GORank::PreparePlayback() {
  m_sender.ResetKey();
  for (unsigned i = 0; i < m_MaxNoteVelocities.size(); i++)
    m_MaxNoteVelocities[i] = 0;
  for (unsigned i = 0; i < m_NoteStopVelocities.size(); i++)
    for (unsigned j = 0; j < m_NoteStopVelocities[i].size(); j++)
      m_NoteStopVelocities[i][j] = 0;
  m_sender.SetName(m_Name);
}

void GORank::SendKey(unsigned note, unsigned velocity) {
  m_sender.SetKey(note, velocity);
}

wxString GORank::GetMidiType() { return _("Rank"); }

wxString GORank::GetMidiName() { return m_Name; }

void GORank::ShowConfigDialog() {
  wxString title = wxString::Format(
    _("Midi-Settings for %s - %s"),
    GetMidiType().c_str(),
    GetMidiName().c_str());

  m_OrganController->ShowMIDIEventDialog(this, title, NULL, &m_sender, NULL);
}

wxString GORank::GetElementStatus() { return _("-"); }

std::vector<wxString> GORank::GetElementActions() {
  std::vector<wxString> actions;
  return actions;
}

void GORank::TriggerElementActions(unsigned no) {
  // Never called
}
