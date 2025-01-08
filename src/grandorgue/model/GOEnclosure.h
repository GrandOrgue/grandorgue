/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOENCLOSURE_H_
#define GOENCLOSURE_H_

#include <cstdint>

#include <wx/string.h>

#include "control/GOControl.h"
#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiSender.h"
#include "midi/GOMidiShortcutReceiver.h"
#include "sound/GOSoundStateHandler.h"

#include "GOEventHandler.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
class GOOrganModel;

class GOEnclosure : public GOControl,
                    private GOEventHandler,
                    private GOSaveableObject,
                    private GOSoundStateHandler,
                    public GOMidiConfigurator {
private:
  GOOrganModel &r_OrganModel;
  GOMidiMap &r_MidiMap;

  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOMidiShortcutReceiver m_shortcut;
  uint8_t m_DefaultAmpMinimumLevel;
  int m_MIDIInputNumber;
  wxString m_Name;
  bool m_Displayed1;
  bool m_Displayed2;

  uint8_t m_AmpMinimumLevel;
  uint8_t m_MIDIValue;

  void ProcessMidi(const GOMidiEvent &event) override;
  void HandleKey(int key) override;

  // Load all customizable values from the .cmb file
  void LoadFromCmb(GOConfigReader &cfg, uint8_t defaultValue);
  void Save(GOConfigWriter &cfg) override;

  void AbortPlayback() override;
  void PreparePlayback() override;
  void PrepareRecording() override;

  GOMidiReceiverBase *GetMidiReceiver() override { return &m_midi; }
  GOMidiSender *GetMidiSender() override { return &m_sender; }
  GOMidiShortcutReceiver *GetMidiShortcutReceiver() override {
    return &m_shortcut;
  }

public:
  GOEnclosure(GOOrganModel &organModel);
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    uint8_t defaultValue);
  void Load(GOConfigReader &cfg, const wxString &group, int enclosure_nb);
  void Set(int n);
  const wxString &GetName() const { return m_Name; }
  int GetValue();
  int GetMIDIInputNumber();
  float GetAttenuation();

  void Scroll(bool scroll_up);
  bool IsDisplayed(bool new_format);
  void SetElementID(int id);

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
  const wxString &GetMidiName() const override { return GetName(); }

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif /* GOENCLOSURE_H_ */
