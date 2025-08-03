/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMetronome.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "control/GOCallbackButtonControl.h"
#include "midi/objects/GOMidiObjectContext.h"
#include "model/GORank.h"
#include "model/GOSoundingPipe.h"
#include "model/GOWindchest.h"

#include "GOOrganController.h"

static const GOMidiObjectContext MIDI_CONTEXT(wxT("Metronome"), _("Metronome"));

const struct GOElementCreator::ButtonDefinitionEntry BUTTON_DEFS[] = {
  {wxT("MetronomeOn"),
   GOMetronome::ID_METRONOME_ON,
   false,
   false,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeMeasureP1"),
   GOMetronome::ID_METRONOME_MEASURE_P1,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeMeasureM1"),
   GOMetronome::ID_METRONOME_MEASURE_M1,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeBpmP1"),
   GOMetronome::ID_METRONOME_BEAT_P1,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeBpmM1"),
   GOMetronome::ID_METRONOME_BEAT_M1,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeBpmP10"),
   GOMetronome::ID_METRONOME_BEAT_P10,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT("MetronomeBpmM10"),
   GOMetronome::ID_METRONOME_BEAT_M10,
   false,
   true,
   false,
   &MIDI_CONTEXT},
  {wxT(""), -1, false, false, false},
};

const GOMetronome::ButtonDefinitionEntry *const GOMetronome::P_BUTTON_DEFS
  = BUTTON_DEFS;

GOMetronome::GOMetronome(GOOrganController *organController)
  : m_OrganController(organController),
    m_BPM(80),
    m_MeasureLength(4),
    m_Pos(0),
    m_Running(false),
    m_BPMDisplay(*organController, &MIDI_CONTEXT),
    m_MeasureDisplay(*organController, &MIDI_CONTEXT),
    m_rank(NULL),
    m_StopID(0) {
  CreateButtons(*m_OrganController, BUTTON_DEFS);
  m_OrganController->RegisterSoundStateHandler(this);
}

GOMetronome::~GOMetronome() { StopTimer(); }

void GOMetronome::Load(GOConfigReader &cfg) {
  m_group = wxT("Metronome");
  m_BPM = cfg.ReadInteger(
    CMBSetting,
    m_group,
    wxT("BPM"),
    1,
    500,
    false,
    m_OrganController->GetSettings().MetronomeBPM());
  m_MeasureLength = cfg.ReadInteger(
    CMBSetting,
    m_group,
    wxT("MeasureLength"),
    0,
    32,
    false,
    m_OrganController->GetSettings().MetronomeMeasure());

  m_buttons[ID_METRONOME_ON]->Init(cfg, wxT("MetronomeOn"), _("ON"));
  m_buttons[ID_METRONOME_MEASURE_P1]->Init(cfg, wxT("MetronomeMP1"), _("+1"));
  m_buttons[ID_METRONOME_MEASURE_M1]->Init(cfg, wxT("MetronomeMM1"), _("-1"));
  m_buttons[ID_METRONOME_BEAT_P1]->Init(cfg, wxT("MetronomeBPMP1"), _("+1"));
  m_buttons[ID_METRONOME_BEAT_M1]->Init(cfg, wxT("MetronomeBPMM1"), _("-1"));
  m_buttons[ID_METRONOME_BEAT_P10]->Init(cfg, wxT("MetronomeBPMP10"), _("+10"));
  m_buttons[ID_METRONOME_BEAT_M10]->Init(cfg, wxT("MetronomeBPMM10"), _("-10"));

  m_BPMDisplay.Init(cfg, wxT("MetronomeBPM"), _("Metronome BPM"));
  m_MeasureDisplay.Init(cfg, wxT("MetronomeMeasure"), _("Metronom measure"));

  m_OrganController->RegisterSaveableObject(this);

  GOWindchest *pWindchest = new GOWindchest(*m_OrganController);

  pWindchest->SetHardName(wxT("Metronome"));
  pWindchest->Init(cfg, wxT("MetronomeWindchest"), _("Metronome"));
  pWindchest->GetPipeConfig().GetPipeConfig().SetPercussiveFromInit(BOOL3_TRUE);
  unsigned windchestN = m_OrganController->AddWindchest(pWindchest);

  m_rank = new GORank(*m_OrganController);
  m_rank->Init(cfg, wxT("MetronomSounds"), _("Metronome"), 36, windchestN);
  m_StopID = m_rank->RegisterStop(NULL);
  m_OrganController->AddRank(m_rank);

  GOSoundingPipe *pipe;
  pipe = new GOSoundingPipe(
    m_OrganController, m_rank, windchestN, 36, 8, 100, 100, false);
  m_rank->AddPipe(pipe);
  pipe->Init(
    cfg, wxT("MetronomSounds"), wxT("A"), wxT("sounds\\metronome\\beat.wv"));
  pipe = new GOSoundingPipe(
    m_OrganController, m_rank, windchestN, 37, 8, 100, 100, false);
  m_rank->AddPipe(pipe);
  pipe->Init(
    cfg,
    wxT("MetronomSounds"),
    wxT("B"),
    wxT("sounds\\metronome\\first_beat.wv"));
}

void GOMetronome::Save(GOConfigWriter &cfg) {
  cfg.WriteInteger(m_group, wxT("BPM"), m_BPM);
  cfg.WriteInteger(m_group, wxT("MeasureLength"), m_MeasureLength);
}

void GOMetronome::ButtonStateChanged(int id, bool newState) {
  switch (id) {
  case ID_METRONOME_ON:
    if (m_Running)
      StopTimer();
    else
      StartTimer();
    break;

  case ID_METRONOME_MEASURE_P1:
    UpdateMeasure(1);
    break;

  case ID_METRONOME_MEASURE_M1:
    UpdateMeasure(-1);
    break;

  case ID_METRONOME_BEAT_P1:
    UpdateBPM(1);
    break;

  case ID_METRONOME_BEAT_M1:
    UpdateBPM(-1);
    break;

  case ID_METRONOME_BEAT_P10:
    UpdateBPM(10);
    break;

  case ID_METRONOME_BEAT_M10:
    UpdateBPM(-10);
    break;
  }
}

void GOMetronome::UpdateBPM(int val) {
  if ((int)m_BPM + val < 1)
    m_BPM = 1;
  else
    m_BPM += val;
  if (m_BPM > 500)
    m_BPM = 500;
  if (m_Running)
    m_OrganController->GetTimer()->UpdateInterval(this, 60000 / m_BPM);
  UpdateState();
}

void GOMetronome::UpdateMeasure(int val) {
  if ((int)m_MeasureLength + val < 0)
    m_MeasureLength = 0;
  else
    m_MeasureLength += val;
  if (m_MeasureLength > 32)
    m_MeasureLength = 32;
  UpdateState();
}

void GOMetronome::UpdateState() {
  m_BPMDisplay.SetContent(wxString::Format(_("%d BPM"), m_BPM));
  m_MeasureDisplay.SetContent(wxString::Format(_("%d"), m_MeasureLength));
  m_buttons[ID_METRONOME_ON]->Display(m_Running);
}

void GOMetronome::StartTimer() {
  StopTimer();
  m_Pos = 0;
  m_Running = true;
  UpdateState();
  m_OrganController->GetTimer()->SetTimer(0, this, 60000 / m_BPM);
}

void GOMetronome::StopTimer() {
  m_OrganController->GetTimer()->DeleteTimer(this);
  m_Running = false;
  UpdateState();
}

void GOMetronome::HandleTimer() {
  unsigned type = 0;
  if (m_Pos || !m_MeasureLength)
    type = 0;
  else
    type = 1;
  m_rank->SetKey(type, 0x7f, m_StopID);
  m_rank->SetKey(type, 0, m_StopID);

  m_Pos++;
  if (m_Pos >= m_MeasureLength)
    m_Pos = 0;
}

void GOMetronome::AbortPlayback() { StopTimer(); }

void GOMetronome::PreparePlayback() {
  m_Running = false;
  UpdateState();
}

GOEnclosure *GOMetronome::GetEnclosure(const wxString &name, bool is_panel) {
  return NULL;
}

GOLabelControl *GOMetronome::GetLabelControl(
  const wxString &name, bool is_panel) {
  if (is_panel)
    return NULL;

  if (name == wxT("MetronomeBPM"))
    return &m_BPMDisplay;
  if (name == wxT("MetronomeMeasure"))
    return &m_MeasureDisplay;

  return NULL;
}
