/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMANUAL_H
#define GOMANUAL_H

#include <wx/string.h>

#include "ptrvector.h"

#include "combinations/control/GOCombinationButtonSet.h"
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
class GOOrganModel;

class GOManual : private GOEventHandler,
                 private GOCombinationButtonSet,
                 private GOSaveableObject,
                 private GOSoundStateHandler,
                 public GOMidiConfigurator {
private:
  GOOrganModel &r_OrganModel;
  GOMidiMap &r_MidiMap;

  wxString m_group;
  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOMidiSender m_division;
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

  // Global Switch Id is the number of switch in ODF started with 1
  std::vector<unsigned> m_GlobalSwitchIds;

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

  /**
   * Update all divisional buttons light.
   * @param buttonToLight
   * @param manualIndexOnlyFor
   */
  void UpdateAllButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor) override;

protected:
  GOMidiReceiverBase *GetMidiReceiver() override { return &m_midi; }
  GOMidiSender *GetMidiSender() override { return &m_sender; }
  GOMidiSender *GetDivision() override { return &m_division; }

public:
  GOManual(GOOrganModel &organModel);

  unsigned GetManulNumber() const { return m_manual_number; }

  void Init(
    GOConfigReader &cfg,
    wxString group,
    int manualNumber,
    unsigned first_midi,
    unsigned keys);
  void Load(GOConfigReader &cfg, const wxString &group, int manualNumber);
  void LoadDivisionals(GOConfigReader &cfg);
  unsigned RegisterCoupler(GOCoupler *coupler);
  // send the key state to all outgoing couplers
  void PropagateKeyToCouplers(unsigned note);
  void SetKey(unsigned note, unsigned velocity, unsigned couplerID);
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

  unsigned GetStopCount() const { return m_stops.size(); };
  GOStop *GetStop(unsigned index);

  /**
   * Find a stop by it's name
   * @param name - the name of stop to find
   * @return the stop index or -1 if the stop is not found
   */
  int FindStopByName(const wxString &name) const;
  unsigned GetCouplerCount() const { return m_couplers.size(); }
  unsigned GetODFCouplerCount() const { return m_ODFCouplerCount; }
  GOCoupler *GetCoupler(unsigned index);
  /**
   * Find a coupler by it's name
   * @param name - the name of coupler to find
   * @return the stop index or -1 if the stop is not found
   */
  int FindCouplerByName(const wxString &name) const;
  void AddCoupler(GOCoupler *coupler);
  unsigned GetDivisionalCount() const { return m_divisionals.size(); }
  GODivisionalButtonControl *GetDivisional(unsigned index);
  void AddDivisional(GODivisionalButtonControl *divisional);
  unsigned GetTremulantCount() const { return m_tremulant_ids.size(); }
  GOTremulant *GetTremulant(unsigned index);
  /**
   * Find a tremulant belonging to this manual by it's name
   * @param name - the name of tremulant to find
   * @return the tremulant index in the OrganModel (not in the manual) or -1
   *   if the tremulant is not found
   */
  int FindTremulantByName(const wxString &name) const;
  unsigned GetSwitchCount() const { return m_GlobalSwitchIds.size(); }
  GOSwitch *GetSwitch(unsigned index);
  /**
   * Find a switch belonging to this manual by it's name
   * @param name - the name of switch to find
   * @return the switch index in the OrganModel (not in the manual) or -1
   *   if the switch is not found
   */
  int FindSwitchByName(const wxString &name) const;

  GOCombinationDefinition &GetDivisionalTemplate();
  const wxString &GetName() const { return m_name; }
  bool IsDisplayed();
  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
  const wxString &GetMidiName() const override { return GetName(); }

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif
