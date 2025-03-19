/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLABELCONTROL_H
#define GOLABELCONTROL_H

#include <wx/string.h>

#include "midi/objects/GOMidiSendingObject.h"

#include "GOControl.h"

class GOMidiObjectContext;
class GOOrganModel;

class GOLabelControl : public GOControl, public GOMidiSendingObject {
protected:
  wxString m_Content;

  void SendCurrentMidiValue() override { SendMidiValue(m_Content); }
  void SendEmptyMidiValue() override { SendMidiValue(wxEmptyString); }

public:
  GOLabelControl(
    GOOrganModel &organModel, const GOMidiObjectContext *pContext = nullptr);
  const wxString &GetContent() const { return m_Content; }
  void SetContent(const wxString &name);

  wxString GetElementStatus() override { return m_Content; }
  std::vector<wxString> GetElementActions() override {
    return std::vector<wxString>();
  }
  void TriggerElementActions(unsigned no) override { /* Never called */
  }
};

#endif
