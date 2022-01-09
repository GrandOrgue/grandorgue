/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPUSHBUTTON_H
#define GOPUSHBUTTON_H

#include "GOButton.h"

class GOPushbutton : public GOButton {
public:
  GOPushbutton(GODefinitionFile *organfile);
};

#endif
