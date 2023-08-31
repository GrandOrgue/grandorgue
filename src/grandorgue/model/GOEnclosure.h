/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOENCLOSURE_H_
#define GOENCLOSURE_H_

#include <wx/string.h>

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

class GOEnclosure : private GOEventHandler,
                    private GOSaveableObject,
                    private GOSoundStateHandler,
                    public GOMidiConfigurator {
private:
  GOOrganModel &r_OrganModel;
  GOMidiMap &r_MidiMap;

  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOMidiShortcutReceiver m_shortcut;
  int m_AmpMinimumLevel;
  int m_MIDIInputNumber;
  int m_MIDIValue;
  wxString m_Name;
  bool m_Displayed1;
  bool m_Displayed2;

  void ProcessMidi(const GOMidiEvent &event);
  void HandleKey(int key);

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();
  void PrepareRecording();

  GOMidiReceiverBase *GetMidiReceiver() override { return &m_midi; }
  GOMidiSender *GetMidiSender() override { return &m_sender; }
  GOMidiShortcutReceiver *GetMidiShortcutReceiver() override {
    return &m_shortcut;
  }

public:
  GOEnclosure(GOOrganModel &organModel);
  void Init(
    GOConfigReader &cfg, wxString group, wxString Name, unsigned def_value);
  void Load(GOConfigReader &cfg, wxString group, int enclosure_nb);
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

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif /* GOENCLOSURE_H_ */
