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

protected:
  virtual GOMidiReceiverBase *GetMidiReceiver() { return nullptr; }
  virtual GOMidiSender *GetMidiSender() { return nullptr; }
  virtual GOMidiShortcutReceiver *GetMidiShortcutReceiver() { return nullptr; }
  virtual GOMidiSender *GetDivision() { return nullptr; }

public:
  GOMidiObject(GOMidiDialogCreator &dialogCreator)
    : r_DialogCreator(dialogCreator) {}

  virtual ~GOMidiObject() {}

  virtual const wxString &GetMidiTypeCode() const = 0;
  virtual const wxString &GetMidiType() const = 0;
  virtual const wxString &GetMidiName() const = 0;

  void ShowConfigDialog();

  virtual wxString GetElementStatus() = 0;
  virtual std::vector<wxString> GetElementActions() = 0;
  virtual void TriggerElementActions(unsigned no) = 0;
};

#endif