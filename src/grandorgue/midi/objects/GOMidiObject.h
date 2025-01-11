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

class GOMidiDialogCreator;
class GOMidiReceiverBase;
class GOMidiSender;
class GOMidiShortcutReceiver;

class GOMidiObject {
private:
  GOMidiDialogCreator &r_DialogCreator;
  const wxString &r_MidiTypeCode;
  const wxString &r_MidiTypeName;
  const wxString &r_MidiName;

  GOMidiSender *p_MidiSender;
  GOMidiReceiverBase *p_MidiReceiver;
  GOMidiShortcutReceiver *p_ShortcutReceiver;
  GOMidiSender *p_DivisionSender;

protected:
  GOMidiObject(
    GOMidiDialogCreator &dialogCreator,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    const wxString &midiName,
    GOMidiSender *pMidiSender,
    GOMidiReceiverBase *pMidiReceiver,
    GOMidiShortcutReceiver *pShortcutReceiver,
    GOMidiSender *pDivisionSender);

  virtual ~GOMidiObject() {}

public:
  const wxString &GetMidiTypeCode() const { return r_MidiTypeCode; }
  const wxString &GetMidiTypeName() const { return r_MidiTypeName; }
  const wxString &GetMidiName() const { return r_MidiName; }

  virtual bool IsReadOnly() const { return false; }

  void ShowConfigDialog();

  // Used in the GOMidiList dialog
  virtual wxString GetElementStatus() = 0;
  virtual std::vector<wxString> GetElementActions() = 0;
  virtual void TriggerElementActions(unsigned no) = 0;
};

#endif