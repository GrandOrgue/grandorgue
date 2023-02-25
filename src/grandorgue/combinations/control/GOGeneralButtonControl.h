/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGENERALBUTTONCONTROL_H
#define GOGENERALBUTTONCONTROL_H

#include "combinations/model/GOGeneralCombination.h"
#include "control/GOPushbuttonControl.h"

class GOConfigReader;

class GOGeneralButtonControl : public GOPushbuttonControl {
private:
  GOGeneralCombination m_general;

public:
  GOGeneralButtonControl(
    GOCombinationDefinition &general_template,
    GOOrganController *organController,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
  void Push();
  GOGeneralCombination &GetGeneral();

  wxString GetMidiType();
};

#endif /* GOGENERAL_H */
