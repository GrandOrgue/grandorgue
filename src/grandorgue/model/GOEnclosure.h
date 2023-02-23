/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOENCLOSURE_H_
#define GOENCLOSURE_H_

#include <wx/string.h>

#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiSender.h"
#include "sound/GOSoundStateHandler.h"

#include "GOEventHandler.h"
#include "GOKeyReceiver.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOOrganController;

class GOEnclosure : private GOEventHandler,
                    private GOSaveableObject,
                    private GOSoundStateHandler,
                    public GOMidiConfigurator {
private:
  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOKeyReceiver m_shortcut;
  GOOrganController *m_OrganController;
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

public:
  GOEnclosure(GOOrganController *organController);
  virtual ~GOEnclosure();
  void Init(
    GOConfigReader &cfg, wxString group, wxString Name, unsigned def_value);
  void Load(GOConfigReader &cfg, wxString group, int enclosure_nb);
  void Set(int n);
  const wxString &GetName();
  int GetValue();
  int GetMIDIInputNumber();
  float GetAttenuation();

  void Scroll(bool scroll_up);
  bool IsDisplayed(bool new_format);
  void SetElementID(int id);

  wxString GetMidiType();
  wxString GetMidiName();
  void ShowConfigDialog();

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif /* GOENCLOSURE_H_ */
