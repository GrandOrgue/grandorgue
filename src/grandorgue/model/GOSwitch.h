/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSWITCH_H
#define GOSWITCH_H

#include "GODrawStop.h"

class GOSwitch : public GODrawstop {
protected:
  void ChangeState(bool);

public:
  GOSwitch(GOOrganModel &organModel);
  ~GOSwitch();

  wxString GetMidiType();
};

#endif
