/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGENERALBUTTON_H
#define GOGENERALBUTTON_H

#include "combinations/model/GOGeneralCombination.h"

#include "GOPushbutton.h"

class GOConfigReader;

class GOGeneralButton : public GOPushbutton {
private:
  GOGeneralCombination m_general;

public:
  GOGeneralButton(
    GOCombinationDefinition &general_template,
    GODefinitionFile *organfile,
    bool is_setter);
  void Load(GOConfigReader &cfg, wxString group);
  void Push();
  GOGeneralCombination &GetGeneral();

  wxString GetMidiType();
};

#endif /* GOGENERAL_H */
