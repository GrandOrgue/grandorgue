/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOStop.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"

#include "GOOrganModel.h"
#include "GORank.h"

GOStop::GOStop(GOOrganModel &organModel, unsigned first_midi_note_number)
  : GODrawstop(organModel),
    m_RankInfo(0),
    m_KeyVelocity(0),
    m_FirstMidiNoteNumber(first_midi_note_number),
    m_FirstAccessiblePipeLogicalKeyNumber(0),
    m_NumberOfAccessiblePipes(0) {}

unsigned GOStop::IsAuto() const {
  /* m_auto seems to state that if a stop only has 1 note, the note isn't
   * actually controlled by a manual, but will be on if the stop is on and
   * off if the stop is off... */
  return (m_RankInfo.size() == 1 && m_RankInfo[0].Rank->GetPipeCount() == 1);
}

void GOStop::Load(GOConfigReader &cfg, wxString group) {
  unsigned number_of_ranks = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfRanks"), 0, 999, false, 0);

  m_FirstAccessiblePipeLogicalKeyNumber = cfg.ReadInteger(
    ODFSetting, group, wxT("FirstAccessiblePipeLogicalKeyNumber"), 1, 128);
  m_NumberOfAccessiblePipes = cfg.ReadInteger(
    ODFSetting, group, wxT("NumberOfAccessiblePipes"), 1, 192);

  if (number_of_ranks) {
    for (unsigned i = 0; i < number_of_ranks; i++) {
      RankInfo info;
      unsigned no = cfg.ReadInteger(
        ODFSetting,
        group,
        wxString::Format(wxT("Rank%03d"), i + 1),
        1,
        r_OrganModel.GetODFRankCount());
      info.Rank = r_OrganModel.GetRank(no - 1);
      info.FirstPipeNumber = cfg.ReadInteger(
        ODFSetting,
        group,
        wxString::Format(wxT("Rank%03dFirstPipeNumber"), i + 1),
        1,
        info.Rank->GetPipeCount(),
        false,
        1);
      info.PipeCount = cfg.ReadInteger(
        ODFSetting,
        group,
        wxString::Format(wxT("Rank%03dPipeCount"), i + 1),
        1,
        info.Rank->GetPipeCount() - info.FirstPipeNumber + 1,
        false,
        info.Rank->GetPipeCount() - info.FirstPipeNumber + 1);
      info.FirstAccessibleKeyNumber = cfg.ReadInteger(
        ODFSetting,
        group,
        wxString::Format(wxT("Rank%03dFirstAccessibleKeyNumber"), i + 1),
        1,
        m_NumberOfAccessiblePipes,
        false,
        1);
      info.StopID = info.Rank->RegisterStop(this);
      m_RankInfo.push_back(info);
    }
  } else {
    RankInfo info;
    info.Rank = new GORank(r_OrganModel);
    r_OrganModel.AddRank(info.Rank);
    info.FirstPipeNumber = cfg.ReadInteger(
      ODFSetting, group, wxT("FirstAccessiblePipeLogicalPipeNumber"), 1, 192);
    info.FirstAccessibleKeyNumber = 1;
    info.PipeCount = m_NumberOfAccessiblePipes;
    info.Rank->Load(
      cfg,
      group,
      m_FirstMidiNoteNumber - info.FirstPipeNumber
        + info.FirstAccessibleKeyNumber + m_FirstAccessiblePipeLogicalKeyNumber
        - 1);
    info.StopID = info.Rank->RegisterStop(this);
    m_RankInfo.push_back(info);
  }

  m_KeyVelocity.resize(m_NumberOfAccessiblePipes);
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);
  GODrawstop::Load(cfg, group);
}

void GOStop::SetRankKey(unsigned key, unsigned velocity) {
  for (unsigned j = 0; j < m_RankInfo.size(); j++) {
    if (
      key + 1 < m_RankInfo[j].FirstAccessibleKeyNumber
      || key
        >= m_RankInfo[j].FirstAccessibleKeyNumber + m_RankInfo[j].PipeCount)
      continue;
    m_RankInfo[j].Rank->SetKey(
      key + m_RankInfo[j].FirstPipeNumber
        - m_RankInfo[j].FirstAccessibleKeyNumber,
      velocity,
      m_RankInfo[j].StopID);
  }
}

void GOStop::SetKey(unsigned note, unsigned velocity) {
  if (
    note < m_FirstAccessiblePipeLogicalKeyNumber
    || note
      >= m_FirstAccessiblePipeLogicalKeyNumber + m_NumberOfAccessiblePipes)
    return;
  if (IsAuto())
    return;
  note -= m_FirstAccessiblePipeLogicalKeyNumber;

  if (m_KeyVelocity[note] == velocity)
    return;
  m_KeyVelocity[note] = velocity;
  if (IsActive())
    SetRankKey(note, m_KeyVelocity[note]);
}

void GOStop::ChangeState(bool on) {
  if (IsAuto()) {
    SetRankKey(0, on ? 0x7f : 0x00);
  } else {
    for (unsigned i = 0; i < m_NumberOfAccessiblePipes; i++)
      SetRankKey(i, on ? m_KeyVelocity[i] : 0);
  }
}

GOStop::~GOStop(void) {}

void GOStop::AbortPlayback() {
  if (IsAuto())
    Set(false);
  GOButtonControl::AbortPlayback();
}

void GOStop::PreparePlayback() {
  GODrawstop::PreparePlayback();

  m_KeyVelocity.resize(m_NumberOfAccessiblePipes);
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);
}

void GOStop::StartPlayback() {
  GODrawstop::StartPlayback();

  if (IsAuto() && IsActive())
    SetRankKey(0, 0x7f);
}

GORank *GOStop::GetRank(unsigned index) { return m_RankInfo[index].Rank; }

const wxString WX_MIDI_TYPE_CODE = wxT("Stop");
const wxString WX_MIDI_TYPE = _("Stop");

const wxString &GOStop::GetMidiTypeCode() const { return WX_MIDI_TYPE_CODE; }

const wxString &GOStop::GetMidiType() const { return WX_MIDI_TYPE; }
