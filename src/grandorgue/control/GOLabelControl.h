/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLABELCONTROL_H
#define GOLABELCONTROL_H

#include <wx/string.h>

#include "midi/GOMidiSender.h"
#include "midi/objects/GOMidiSendingObject.h"
#include "sound/GOSoundStateHandler.h"

#include "GOControl.h"
#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganModel;

class GOLabelControl : public GOMidiSendingObject, public GOControl {
protected:
  GOOrganModel &r_OrganModel;
  wxString m_Content;

  void AbortPlayback() override;
  void PrepareRecording() override;

public:
  GOLabelControl(GOOrganModel &organModel);
  const wxString &GetContent();
  void SetContent(wxString name);

  wxString GetElementStatus() override;
  std::vector<wxString> GetElementActions() override;
  void TriggerElementActions(unsigned no) override;
};

#endif
