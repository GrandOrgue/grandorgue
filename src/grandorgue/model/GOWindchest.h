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
  /**
   * Non-owning back references to the ranks of this windchest. They are never
   * removed one by one, see AddRank()
   */
  std::vector<GORank *> m_ranks;
  /**
   * Non-owning back references to the pipes of this windchest. They are never
   * removed one by one, see AddPipe()
   */
  std::vector<GOPipeWindchestCallback *> m_pipes;
  GOPipeConfigTreeNode m_PipeConfig;

  void PreparePlayback();

public:
  GOWindchest(GOOrganModel &organModel);

  /** Unregisters the lifecycle listener registered by the constructor. */
  ~GOWindchest();

  const wxString &GetHardName() const { return m_HardName; }
  void SetHardName(const wxString &name) { m_HardName = name; }

  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group, unsigned index);
  void UpdateTremulant(GOTremulant *tremulant);
  void UpdateVolume();
  float GetAmplitude() const { return m_amplitude; }
  unsigned GetTremulantCount() const { return m_tremulant.size(); }
  unsigned GetTremulantId(unsigned index) const { return m_tremulant[index]; }
  /**
   * Attaches an already added tremulant to this windchest. Load() does the same
   * from the ODF; this method is for the code that builds an organ without an
   * ODF (the tests).
   *
   * @param id the index of the tremulant in GOOrganModel
   */
  void AddTremulant(unsigned id) { m_tremulant.push_back(id); }
  /**
   * Detaches the tremulant attached by AddTremulant(). The mirror of
   * AddTremulant(): the tremulant must be the last attached one (an assertion
   * failure in Debug; ignored in Release if it is not the last one).
   *
   * @param id the index of the tremulant in GOOrganModel
   */
  void RemoveTremulant(unsigned id);
  unsigned GetRankCount() const { return m_ranks.size(); }
  GORank *GetRank(unsigned index) const {
    return index < m_ranks.size() ? m_ranks[index] : nullptr;
  }
  /**
   * Adds the back reference from the windchest to its rank (called by the
   * rank). It is intentionally not mirrored by a Remove: the windchest is
   * deleted after its ranks (GOMetronome::ClearLoaded() removes the rank
   * before ClearOrganCoreData() removes the windchest; GOOrganModel::Cleanup()
   * deletes the ranks before the windchests). Between the deleting of the
   * ranks and the windchest the list holds dangling pointers, but it is not
   * read then (the sound is stopped) and the destructor of the windchest does
   * not touch it. A Remove for every rank would make closing an organ O(n^2).
   * @param rank the non-owning pointer to the rank
   */
  void AddRank(GORank *rank) { m_ranks.push_back(rank); }
  /**
   * Adds the back reference from the windchest to its pipe (called by the
   * pipe). It is intentionally not mirrored by a Remove, for the same reasons
   * as AddRank().
   * @param pipe the non-owning pointer to the pipe
   */
  void AddPipe(GOPipeWindchestCallback *pipe) { m_pipes.push_back(pipe); }
  void AddEnclosure(GOEnclosure *enclosure) {
    m_enclosure.push_back(enclosure);
  }
  /**
   * Removes the enclosure added by AddEnclosure(); does not delete it. The
   * mirror of AddEnclosure(): the enclosure must be the last added one (an
   * assertion failure in Debug; ignored in Release if it is not the last one).
   * @param pEnclosure the enclosure to remove
   */
  void RemoveEnclosure(GOEnclosure *pEnclosure);
  const std::vector<GOEnclosure *> &GetEnclosures() const {
    return m_enclosure;
  }
  const wxString &GetName() const { return m_Name; }
  GOPipeConfigNode &GetPipeConfig() { return m_PipeConfig; }
};

#endif /* GOWINDCHEST_H_ */
