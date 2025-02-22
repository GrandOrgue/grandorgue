/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECT_H
#define GOMIDIOBJECT_H

#include <wx/string.h>

#include <vector>

#include "midi/dialog-creator/GOMidiDialogListener.h"
#include "sound/GOSoundStateHandler.h"

#include "GOSaveableObject.h"

class GOMidiMap;
class GOMidiReceiver;
class GOMidiSender;
class GOMidiShortcutReceiver;
class GOOrganModel;

class GOMidiObject : public GOSoundStateHandler,
                     public GOSaveableObject,
                     protected GOMidiDialogListener {
protected:
  GOOrganModel &r_OrganModel;

private:
  GOMidiMap &r_MidiMap;
  const wxString &r_MidiTypeCode;
  const wxString &r_MidiTypeName;

  wxString m_name;

  GOMidiSender *p_MidiSender;
  GOMidiReceiver *p_MidiReceiver;
  GOMidiShortcutReceiver *p_ShortcutReceiver;
  GOMidiSender *p_DivisionSender;

protected:
  GOMidiObject(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName);

  virtual ~GOMidiObject();

  GOMidiSender *GetMidiSender() const { return p_MidiSender; }
  void SetMidiSender(GOMidiSender *pMidiSender) { p_MidiSender = pMidiSender; }
  GOMidiReceiver *GetMidiReceiver() const { return p_MidiReceiver; }
  void SetMidiReceiver(GOMidiReceiver *pMidiReceiver) {
    p_MidiReceiver = pMidiReceiver;
  }
  GOMidiShortcutReceiver *GetMidiShortcutReceiver() const {
    return p_ShortcutReceiver;
  }
  void SetMidiShortcutReceiver(GOMidiShortcutReceiver *pShortcutReceiver) {
    p_ShortcutReceiver = pShortcutReceiver;
  }
  GOMidiSender *GetDivisionSender() const { return p_DivisionSender; }
  void SetDivisionSender(GOMidiSender *pDivisionSender) {
    p_DivisionSender = pDivisionSender;
  }

private:
  void InitMidiObject(
    GOConfigReader &cfg, const wxString &group, const wxString &name);

protected:
  virtual void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) {}
  virtual void SaveMidiObject(
    GOConfigWriter &cfg, const wxString &group, GOMidiMap &midiMap) {}

public:
  GOMidiMap &GetMidiMap() { return r_MidiMap; }
  const wxString &GetMidiTypeCode() const { return r_MidiTypeCode; }
  const wxString &GetMidiTypeName() const { return r_MidiTypeName; }
  const wxString &GetName() const { return m_name; }
  void SetName(const wxString &name) { m_name = name; }

  virtual void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) {
    InitMidiObject(cfg, group, name);
  }

  virtual void Load(
    GOConfigReader &cfg, const wxString &group, const wxString &name) {
    InitMidiObject(cfg, group, name);
  }

  virtual void Save(GOConfigWriter &cfg) {
    SaveMidiObject(cfg, m_group, r_MidiMap);
  }

  virtual bool IsReadOnly() const { return false; }

  void PreparePlayback() override {}
  void PrepareRecording() override {}
  void AbortPlayback() override {}

  void ShowConfigDialog();

  // Used in the GOMidiList dialog
  virtual wxString GetElementStatus() = 0;
  virtual std::vector<wxString> GetElementActions() = 0;
  virtual void TriggerElementActions(unsigned no) = 0;
};

#endif