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
#include "midi/objects/GOMidiObjectWithShortcut.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganModel;

class GOEnclosure : public GOControl, public GOMidiObjectWithShortcut {
private:
  bool m_IsOdfDefined;
  uint8_t m_DefaultAmpMinimumLevel;
  uint8_t m_MIDIInputNumber;
  bool m_Displayed1;
  bool m_Displayed2;

  uint8_t m_AmpMinimumLevel;
  uint8_t m_MIDIValue;

  void OnMidiReceived(
    const GOMidiEvent &event,
    GOMidiMatchType matchType,
    int key,
    int value) override;
  void OnShortcutKeyReceived(
    GOMidiShortcutReceiver::MatchType matchType, int key) override;

  // Load all customizable values from the .cmb file
  void LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue);
  void Save(GOConfigWriter &cfg) override;

  void SendCurrentMidiValue() override { SendMidiValue(m_MIDIValue); }
  void SendEmptyMidiValue() override { SendMidiValue(0); }

  void SetIntEnclosureValue(int n) { SetEnclosureValue(std::clamp(n, 0, 127)); }

public:
  static constexpr uint8_t MAX_MIDI_VALUE = 127;

  GOEnclosure(GOOrganModel &organModel);

  bool IsOdfDefined() const { return m_IsOdfDefined; }

  using GOMidiObjectWithShortcut::Init; // for avoiding a warning
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    uint8_t defValue);
  using GOMidiObject::Load; // for avoiding a warning
  void Load(GOConfigReader &cfg, const wxString &group, int enclosureNb);
  uint8_t GetDefaultAmpMinimumLevel() const { return m_DefaultAmpMinimumLevel; }
  uint8_t GetAmpMinimumLevel() const { return m_AmpMinimumLevel; }
  void SetAmpMinimumLevel(uint8_t v) { m_AmpMinimumLevel = v; }
  void SetEnclosureValue(uint8_t n);
  int GetEnclosureValue() const { return m_MIDIValue; }
  int GetMIDIInputNumber() const { return m_MIDIInputNumber; }
  float GetAttenuation();

  void Scroll(bool scroll_up);
  bool IsDisplayed(bool new_format);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif /* GOENCLOSURE_H_ */