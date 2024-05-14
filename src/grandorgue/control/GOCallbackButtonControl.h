/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCALLBACKBUTTONCONTROL_H
#define GOCALLBACKBUTTONCONTROL_H

#include "GOButtonControl.h"

class GOButtonCallback;

class GOCallbackButtonControl : public GOButtonControl {
protected:
  GOButtonCallback *m_callback;

public:
  GOCallbackButtonControl(
    GOOrganModel &organModel,
    GOButtonCallback *callback,
    bool isPushbutton,
    bool isPiston = false);
  void Push(void);
  void Set(bool on);

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif
