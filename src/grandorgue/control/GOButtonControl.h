/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUTTONCONTROL_H
#define GOBUTTONCONTROL_H

#include <wx/string.h>

#include "midi/GOMidiReceiver.h"
#include "midi/GOMidiSender.h"
#include "midi/GOMidiShortcutReceiver.h"
#include "midi/objects/GOMidiSendingObject.h"
#include "sound/GOSoundStateHandler.h"

#include "GOControl.h"
#include "GOEventHandler.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
class GOOrganModel;

class GOButtonControl : public GOMidiSendingObject,
                        public GOControl,
                        private GOEventHandler {
protected:
  GOOrganModel &r_OrganModel;
  GOMidiReceiver m_midi;
  GOMidiShortcutReceiver m_shortcut;
  bool m_Pushbutton;
  bool m_Displayed;
  bool m_Engaged;
  bool m_DisplayInInvertedState;
  bool m_ReadOnly;
  bool m_IsPiston;

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) override;

  void ProcessMidi(const GOMidiEvent &event) override;
  void HandleKey(int key) override;

  void PreparePlayback() override;
  void PrepareRecording() override;
  void AbortPlayback() override;

public:
  GOButtonControl(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiReceiverType midi_type,
    bool pushbutton,
    bool isPiston = false);
  ~GOButtonControl();
  void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) override;
  using GOMidiObject::Load; // Avoiding a compilation warning
  virtual void Load(GOConfigReader &cfg, const wxString &group);
  bool IsDisplayed();
  void SetDisplayed(bool displayed) { m_Displayed = displayed; }
  bool IsReadOnly() const override { return m_ReadOnly; }
  bool IsPiston() const { return m_IsPiston; }

  virtual void Push();
  virtual void SetButtonState(bool on);
  virtual void Display(bool onoff);
  bool IsEngaged() const;
  bool DisplayInverted() const;
  void SetElementId(int id) override;
  void SetShortcutKey(unsigned key);
  void SetPreconfigIndex(unsigned index);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif
