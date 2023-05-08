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
class GOSetter;

class GOGeneralButtonControl : public GOPushbuttonControl {
private:
  GOSetter &r_setter;
  GOGeneralCombination m_combination;

public:
  GOGeneralButtonControl(
    GOCombinationDefinition &general_template,
    GOOrganController *organController,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
  void Push();
  GOGeneralCombination &GetCombination();

  wxString GetMidiType();
};

#endif /* GOGENERAL_H */
