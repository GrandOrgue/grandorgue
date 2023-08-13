/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPISTONCONTROL_H
#define GOPISTONCONTROL_H

#include "GOControlChangedHandler.h"
#include "GOPushbuttonControl.h"

class GOButtonControl;
class GOConfigReader;
class GOConfigWriter;

class GOPistonControl : public GOPushbuttonControl,
                        private GOControlChangedHandler {
private:
  GOButtonControl *drawstop;

  void ControlChanged(void *control);

public:
  GOPistonControl(GOOrganModel &organModel);
  void Load(GOConfigReader &cfg, wxString group);
  void Push();

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif /* GOPISTON_H */
