/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOBUTTONCONTROL_H
#define GOBUTTONCONTROL_H

#include <wx/string.h>

#include "midi/objects/GOMidiObjectWithShortcut.h"

#include "GOControl.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiEvent;
class GOMidiMap;
class GOOrganModel;

class GOButtonControl : public GOControl, public GOMidiObjectWithShortcut {
protected:
  bool m_Pushbutton;
  bool m_Displayed;
  bool m_Engaged;
  bool m_DisplayInInvertedState;
  bool m_ReadOnly;
  bool m_IsPiston;

  void OnMidiReceived(
    const GOMidiEvent &event,
    GOMidiMatchType matchType,
    int key,
    int value) override;
  void OnShortcutKeyReceived(
    GOMidiShortcutReceiver::MatchType matchType, int key) override;

  void SendCurrentMidiValue() override { SendMidiValue(m_Engaged); }
  void SendEmptyMidiValue() override { SendMidiValue(false); }

public:
  GOButtonControl(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiReceiverType midiType,
    bool pushbutton,
    bool isPiston = false);

  void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) override;
  using GOMidiObject::Load; // Avoiding a compilation warning
  virtual void Load(GOConfigReader &cfg, const wxString &group);

  bool IsDisplayed() const { return m_Displayed; }
  void SetDisplayed(bool displayed) { m_Displayed = displayed; }
  bool IsReadOnly() const override { return m_ReadOnly; }
  bool IsPiston() const { return m_IsPiston; }
  bool IsEngaged() const { return m_Engaged; }
  bool DisplayInverted() const { return m_DisplayInInvertedState; }

  virtual void Push();
  virtual void SetButtonState(bool on) {}
  virtual void Display(bool onoff);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif
