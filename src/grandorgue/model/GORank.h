/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GORANK_H
#define GORANK_H

#include "ptrvector.h"

#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiSender.h"
#include "pipe-config/GOPipeConfigTreeNode.h"
#include "sound/GOSoundStateHandler.h"

#include "GOSaveableObject.h"

class GOMidiMap;
class GOOrganModel;
class GOPipe;
class GOStop;
class GOTemperament;

class GORank : private GOSaveableObject,
               public GOMidiConfigurator,
               private GOSoundStateHandler {
private:
  GOOrganModel &r_OrganModel;
  GOMidiMap &r_MidiMap;
  wxString m_Name;
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
  GOMidiSender m_sender;
  GOPipeConfigTreeNode m_PipeConfig;

  void Resize();

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();

protected:
  GOMidiSender *GetMidiSender() override { return &m_sender; }

public:
  GORank(GOOrganModel &organModel);
  ~GORank();
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
  void Load(
    GOConfigReader &cfg, const wxString &group, int defaultFirstMidiNoteNumber);
  void AddPipe(GOPipe *pipe);
  unsigned RegisterStop(GOStop *stop);
  void SetKey(int note, unsigned velocity, unsigned stopID);
  GOPipe *GetPipe(unsigned index);
  unsigned GetPipeCount();
  GOPipeConfigNode &GetPipeConfig();
  void SetTemperament(const GOTemperament &temperament);
  const wxString &GetName() const { return m_Name; }

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
  const wxString &GetMidiName() const override { return GetName(); }

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);

  void SendKey(unsigned note, unsigned velocity);
};

#endif
