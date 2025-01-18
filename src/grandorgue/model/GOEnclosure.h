/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOENCLOSURE_H_
#define GOENCLOSURE_H_

#include <algorithm>
#include <cstdint>

#include <wx/string.h>

#include "control/GOControl.h"
#include "midi/GOMidiShortcutReceiver.h"
#include "midi/objects/GOMidiReceivingSendingObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;
class GOOrganModel;

class GOEnclosure : public GOControl, public GOMidiReceivingSendingObject {
private:
  GOMidiShortcutReceiver m_shortcut;
  wxString m_Name;
  uint8_t m_DefaultAmpMinimumLevel;
  uint8_t m_MIDIInputNumber;
  bool m_Displayed1;
  bool m_Displayed2;

  uint8_t m_AmpMinimumLevel;
  uint8_t m_MIDIValue;

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) override;

  void OnMidiReceived(
    const GOMidiEvent &event,
    GOMidiMatchType matchType,
    int key,
    int value) override;
  void HandleKey(int key) override;

  // Load all customizable values from the .cmb file
  void LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue);
  void Save(GOConfigWriter &cfg) override;

  void PrepareRecording() override;
  void AbortPlayback() override;

public:
  static constexpr uint8_t MAX_MIDI_VALUE = 127;

  GOEnclosure(GOOrganModel &organModel);
  ~GOEnclosure();

  using GOMidiReceivingSendingObject::Init; // for avoiding a warning
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    uint8_t defValue);
  using GOMidiObject::Load; // for avoiding a warning
  void Load(GOConfigReader &cfg, const wxString &group, int enclosureNb);
  void SetEnclosureValue(int n);
  int GetEnclosureValue() const { return m_MIDIValue; }
  int GetMIDIInputNumber();
  float GetAttenuation();

  void Scroll(bool scroll_up);
  bool IsDisplayed(bool new_format);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif /* GOENCLOSURE_H_ */
