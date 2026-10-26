/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOWINDCHEST_H
#define GOWINDCHEST_H

#include <vector>

#include <wx/string.h>

#include "pipe-config/GOPipeConfigTreeNode.h"

#include "GOOrganLifecycleListener.h"

class GOConfigReader;
class GOEnclosure;
class GOPipeWindchestCallback;
class GORank;
class GOTremulant;
class GOOrganModel;

class GOWindchest : private GOOrganLifecycleListener {
private:
  GOOrganModel &r_OrganModel;
  wxString m_Name;
  // a name tat is never translated. May be empty that causes that m_name is
  // used
  wxString m_HardName;

  float m_amplitude;
  std::vector<GOEnclosure *> m_enclosure;
  std::vector<unsigned> m_tremulant;
  std::vector<GORank *> m_ranks;
  std::vector<GOPipeWindchestCallback *> m_pipes;
  GOPipeConfigTreeNode m_PipeConfig;

  void PreparePlayback();

public:
  GOWindchest(GOOrganModel &organModel);

  const wxString &GetHardName() const { return m_HardName; }
  void SetHardName(const wxString &name) { m_HardName = name; }

  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group, unsigned index);
  void UpdateTremulant(GOTremulant *tremulant);
  void UpdateVolume();
  float GetAmplitude() const { return m_amplitude; }
  unsigned GetTremulantCount() const { return m_tremulant.size(); }
  unsigned GetTremulantId(unsigned index) const { return m_tremulant[index]; }
  unsigned GetRankCount() const { return m_ranks.size(); }
  GORank *GetRank(unsigned index) const {
    return index < m_ranks.size() ? m_ranks[index] : nullptr;
  }
  void AddRank(GORank *rank) { m_ranks.push_back(rank); }
  void AddPipe(GOPipeWindchestCallback *pipe) { m_pipes.push_back(pipe); }
  void AddEnclosure(GOEnclosure *enclosure) {
    m_enclosure.push_back(enclosure);
  }
  const std::vector<GOEnclosure *> &GetEnclosures() const {
    return m_enclosure;
  }
  const wxString &GetName() const { return m_Name; }
  GOPipeConfigNode &GetPipeConfig() { return m_PipeConfig; }
};

#endif /* GOWINDCHEST_H_ */
