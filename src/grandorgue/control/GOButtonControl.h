/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUTTONCONTROL_H
#define GOBUTTONCONTROL_H

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
class GOOrganController;

class GOButtonControl : private GOEventHandler,
                        public GOSaveableObject,
                        protected GOSoundStateHandler,
                        public GOMidiConfigurator {
protected:
  GOOrganController *m_OrganController;
  GOMidiReceiver m_midi;
  GOMidiSender m_sender;
  GOKeyReceiver m_shortcut;
  bool m_Pushbutton;
  bool m_Displayed;
  wxString m_Name;
  bool m_Engaged;
  bool m_DisplayInInvertedState;
  bool m_ReadOnly;
  bool m_IsPiston;

  void ProcessMidi(const GOMidiEvent &event);
  void HandleKey(int key);

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();
  void PrepareRecording();

public:
  GOButtonControl(
    GOOrganController *organController,
    GOMidiReceiverType midi_type,
    bool pushbutton,
    bool isPiston = false);
  virtual ~GOButtonControl();
  void Init(GOConfigReader &cfg, const wxString &group, const wxString &name);
  void Load(GOConfigReader &cfg, const wxString &group);
  bool IsDisplayed();
  void SetDisplayed(bool displayed) { m_Displayed = displayed; }
  bool IsReadOnly();
  const wxString &GetName() const { return m_Name; }
  bool IsPiston() const { return m_IsPiston; }

  virtual void Push();
  virtual void Set(bool on);
  virtual void Display(bool onoff);
  bool IsEngaged() const;
  bool DisplayInverted() const;
  void SetElementID(int id);
  void SetShortcutKey(unsigned key);
  void SetPreconfigIndex(unsigned index);

  wxString GetMidiName();
  void ShowConfigDialog();

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif
