/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMANUAL_H
#define GOMANUAL_H

#include <wx/string.h>

#include "ptrvector.h"

#include "combinations/model/GOCombinationDefinition.h"
#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiSender.h"
#include "sound/GOSoundStateHandler.h"

#include "GOEventHandler.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOCoupler;
class GODivisionalButtonControl;
class GOMidiEvent;
class GOStop;
class GOSwitch;
class GOTremulant;
class GOOrganController;

class GOManual : private GOEventHandler,
                 private GOSaveableObject,
                 private GOSoundStateHandler,
                 public GOMidiConfigurator {
private:
  wxString m_group;
  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOMidiSender m_division;
  GOOrganController *m_OrganController;
  std::vector<GOCoupler *> m_InputCouplers;
  /* Keyboard state */
  std::vector<unsigned> m_KeyVelocity;
  /* Internal state affected by couplers */
  std::vector<unsigned> m_RemoteVelocity;
  std::vector<unsigned> m_Velocity;
  std::vector<unsigned> m_DivisionState;
  std::vector<std::vector<unsigned>> m_Velocities;
  unsigned m_MidiMap[128];
  unsigned m_manual_number;
  unsigned m_first_accessible_logical_key_nb;
  unsigned m_nb_logical_keys;
  unsigned m_first_accessible_key_midi_note_nb;
  unsigned m_nb_accessible_keys;
  unsigned m_UnisonOff;

  int m_MIDIInputNumber;

  std::vector<unsigned> m_tremulant_ids;
  std::vector<unsigned> m_switch_ids;

  wxString m_name;

  ptr_vector<GOStop> m_stops;
  ptr_vector<GOCoupler> m_couplers;
  ptr_vector<GODivisionalButtonControl> m_divisionals;
  unsigned m_ODFCouplerCount;
  bool m_displayed;
  GOCombinationDefinition m_DivisionalTemplate;

  void Resize();

  void ProcessMidi(const GOMidiEvent &event);
  void HandleKey(int key);
  void SetOutput(unsigned note, unsigned velocity);

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();
  void PrepareRecording();

public:
  GOManual(GOOrganController *organController);
  void Init(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    unsigned first_midi,
    unsigned keys);
  void Load(GOConfigReader &cfg, wxString group, int manualNumber);
  unsigned RegisterCoupler(GOCoupler *coupler);
  void SetKey(
    unsigned note, unsigned velocity, GOCoupler *prev, unsigned couplerID);
  void Set(unsigned note, unsigned velocity);
  void SetUnisonOff(bool on);
  void Update();
  void Reset();
  void SetElementID(int id);
  ~GOManual(void);

  unsigned GetNumberOfAccessibleKeys();
  unsigned GetFirstAccessibleKeyMIDINoteNumber();
  int GetFirstLogicalKeyMIDINoteNumber();
  int GetMIDIInputNumber();
  unsigned GetLogicalKeyCount();
  void AllNotesOff();
  bool IsKeyDown(unsigned midiNoteNumber);

  unsigned GetStopCount();
  GOStop *GetStop(unsigned index);

  /**
   * Find a stop by it's name
   * @param stopName - the name of stop to find
   * @return the stop index or -1 if the stop is not found
   */
  int FindStopByName(const wxString &stopName) const;
  unsigned GetCouplerCount();
  unsigned GetODFCouplerCount();
  GOCoupler *GetCoupler(unsigned index);
  void AddCoupler(GOCoupler *coupler);
  unsigned GetDivisionalCount();
  GODivisionalButtonControl *GetDivisional(unsigned index);
  void AddDivisional(GODivisionalButtonControl *divisional);
  unsigned GetTremulantCount();
  GOTremulant *GetTremulant(unsigned index);
  unsigned GetSwitchCount();
  GOSwitch *GetSwitch(unsigned index);

  GOCombinationDefinition &GetDivisionalTemplate();
  const wxString &GetName();
  bool IsDisplayed();

  wxString GetMidiType();
  wxString GetMidiName();
  void ShowConfigDialog();

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif
