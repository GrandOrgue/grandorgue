/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GORANK_H
#define GORANK_H

#include "ptrvector.h"

#include "midi/objects/GOMidiSendingObject.h"
#include "pipe-config/GOPipeConfigTreeNode.h"
#include "sound/GOSoundStateHandler.h"

#include "GOPipe.h"

class GOMidiMap;
class GOOrganModel;
class GOStop;
class GOTemperament;

class GORank : public GOMidiSendingObject {
private:
  GOOrganModel &r_OrganModel;
  ptr_vector<GOPipe> m_Pipes;
  /**
   * Number of stops using this rank
   */
  unsigned m_StopCount;
  /**
   * last pressed velocity of notes and stop
   */
  std::vector<std::vector<unsigned>> m_NoteStopVelocities;
  /**
   * maximum last velocity of notes over all stops
   */
  std::vector<unsigned> m_MaxNoteVelocities;
  unsigned m_FirstMidiNoteNumber;
  unsigned m_WindchestN; // starts with 1
  unsigned m_HarmonicNumber;
  float m_MinVolume;
  float m_MaxVolume;
  bool m_RetuneRank;
  GOPipeConfigTreeNode m_PipeConfig;

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg,
    const wxString &group,
    GOMidiMap &midiMap) const override;

  void Resize();

  void PreparePlayback() override;

public:
  GORank(GOOrganModel &organModel);

  using GOMidiObject::Init; // Avoiding a compilation warning
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    unsigned firstMidiNoteNumber,
    unsigned windchestN);
  /**
   * Loads Rank from odf/cmb
   * @param cfg
   * @param group
   * @param firstMidiNoteNumber. -1 means no default and must be specified in
   *   the ODF
   */
  using GOMidiObject::Load; // Avoiding a compilation warning
  void Load(
    GOConfigReader &cfg, const wxString &group, int defaultFirstMidiNoteNumber);
  void AddPipe(GOPipe *pipe);
  unsigned RegisterStop(GOStop *stop);
  void SetKey(int note, unsigned velocity, unsigned stopID);
  GOPipe *GetPipe(unsigned index);
  unsigned GetPipeCount();
  GOPipeConfigNode &GetPipeConfig();
  void SetTemperament(const GOTemperament &temperament);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;

  void SendKey(unsigned note, unsigned velocity) {
    SendMidiKey(note, velocity);
  }
};

#endif
