/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOWindchest.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"

#include "GOEnclosure.h"
#include "GOOrganController.h"
#include "GOPipeWindchestCallback.h"
#include "GOTremulant.h"

GOWindchest::GOWindchest(GOOrganModel &organModel)
  : r_OrganModel(organModel),
    m_Name(),
    m_Volume(1),
    m_enclosure(0),
    m_tremulant(0),
    m_ranks(0),
    m_pipes(0),
    m_PipeConfig(&organModel.GetRootPipeConfigNode(), &organModel, NULL) {
  organModel.RegisterSoundStateHandler(this);
}

void GOWindchest::Init(GOConfigReader &cfg, wxString group, wxString name) {
  m_enclosure.resize(0);
  m_tremulant.resize(0);
  m_Name = name;
  m_PipeConfig.Init(cfg, group, wxEmptyString);
  m_PipeConfig.SetName(GetName());
}

void GOWindchest::Load(GOConfigReader &cfg, wxString group, unsigned index) {
  unsigned NumberOfEnclosures = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("NumberOfEnclosures"),
    0,
    r_OrganModel.GetEnclosureCount());
  unsigned NumberOfTremulants = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("NumberOfTremulants"),
    0,
    r_OrganModel.GetTremulantCount());

  m_enclosure.resize(0);
  for (unsigned i = 0; i < NumberOfEnclosures; i++) {
    wxString buffer;
    buffer.Printf(wxT("Enclosure%03d"), i + 1);
    m_enclosure.push_back(r_OrganModel.GetEnclosureElement(
      cfg.ReadInteger(
        ODFSetting, group, buffer, 1, r_OrganModel.GetEnclosureCount())
      - 1));
  }

  m_tremulant.resize(0);
  for (unsigned i = 0; i < NumberOfTremulants; i++) {
    wxString buffer;
    buffer.Printf(wxT("Tremulant%03d"), i + 1);
    m_tremulant.push_back(
      cfg.ReadInteger(
        ODFSetting, group, buffer, 1, r_OrganModel.GetTremulantCount())
      - 1);
  }

  m_Name = cfg.ReadStringNotEmpty(
    ODFSetting,
    group,
    wxT("Name"),
    false,
    wxString::Format(_("Windchest %d"), index + 1));
  m_PipeConfig.Load(cfg, group, wxEmptyString);
  m_PipeConfig.SetName(GetName());
}

void GOWindchest::UpdateVolume() {
  float f = 1.0f;
  for (unsigned i = 0; i < m_enclosure.size(); i++)
    f *= m_enclosure[i]->GetAttenuation();
  m_Volume = f;
}

float GOWindchest::GetVolume() { return m_Volume; }

unsigned GOWindchest::GetTremulantCount() { return m_tremulant.size(); }

unsigned GOWindchest::GetTremulantId(unsigned no) { return m_tremulant[no]; }

unsigned GOWindchest::GetRankCount() { return m_ranks.size(); }

GORank *GOWindchest::GetRank(unsigned index) { return m_ranks[index]; }

void GOWindchest::AddRank(GORank *rank) { m_ranks.push_back(rank); }

void GOWindchest::AddPipe(GOPipeWindchestCallback *pipe) {
  m_pipes.push_back(pipe);
}

void GOWindchest::AddEnclosure(GOEnclosure *enclosure) {
  m_enclosure.push_back(enclosure);
}

const wxString &GOWindchest::GetName() { return m_Name; }

GOPipeConfigNode &GOWindchest::GetPipeConfig() { return m_PipeConfig; }

void GOWindchest::UpdateTremulant(GOTremulant *tremulant) {
  for (unsigned i = 0; i < m_tremulant.size(); i++)
    if (tremulant == r_OrganModel.GetTremulant(m_tremulant[i])) {
      GOTremulant *t = r_OrganModel.GetTremulant(m_tremulant[i]);
      if (t->GetTremulantType() != GOWavTrem)
        continue;
      bool on = t->IsActive();
      for (unsigned j = 0; j < m_pipes.size(); j++)
        m_pipes[j]->SetWaveTremulant(on);
      return;
    }
}

void GOWindchest::PreparePlayback() { UpdateVolume(); }
