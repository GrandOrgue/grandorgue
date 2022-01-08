/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GORANK_H
#define GORANK_H

#include "GOPipeConfigTreeNode.h"
#include "GOPlaybackStateHandler.h"
#include "GOSaveableObject.h"
#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiSender.h"
#include "ptrvector.h"

class GOPipe;
class GOStop;
class GOTemperament;
class GODefinitionFile;

class GORank : private GOSaveableObject,
               public GOMidiConfigurator,
               private GOPlaybackStateHandler {
 private:
  GODefinitionFile* m_organfile;
  wxString m_Name;
  ptr_vector<GOPipe> m_Pipes;
  unsigned m_StopCount;
  std::vector<unsigned> m_Velocity;
  std::vector<std::vector<unsigned> > m_Velocities;
  unsigned m_FirstMidiNoteNumber;
  bool m_Percussive;
  unsigned m_WindchestGroup;
  unsigned m_HarmonicNumber;
  float m_PitchCorrection;
  float m_MinVolume;
  float m_MaxVolume;
  bool m_RetuneRank;
  GOMidiSender m_sender;
  GOPipeConfigTreeNode m_PipeConfig;

  void Resize();

  void Save(GOConfigWriter& cfg);

  void AbortPlayback();
  void PreparePlayback();
  void StartPlayback();
  void PrepareRecording();

 public:
  GORank(GODefinitionFile* organfile);
  ~GORank();
  void Init(GOConfigReader& cfg, wxString group, wxString name,
            int first_midi_note_number, unsigned windchest);
  void Load(GOConfigReader& cfg, wxString group, int first_midi_note_number);
  void AddPipe(GOPipe* pipe);
  unsigned RegisterStop(GOStop* stop);
  void SetKey(int note, unsigned velocity, unsigned stopID);
  GOPipe* GetPipe(unsigned index);
  unsigned GetPipeCount();
  GOPipeConfigNode& GetPipeConfig();
  void SetTemperament(const GOTemperament& temperament);
  const wxString& GetName();

  wxString GetMidiType();
  wxString GetMidiName();
  void ShowConfigDialog();

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);

  void SendKey(unsigned note, unsigned velocity);
};

#endif
