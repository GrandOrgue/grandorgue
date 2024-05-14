/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODUMMYPIPE_H
#define GODUMMYPIPE_H

#include "GOPipe.h"

class GODummyPipe : public GOPipe {
private:
  void VelocityChanged(unsigned velocity, unsigned old_velocity) override {}

public:
  GODummyPipe(
    GOEventHandlerList *handlerList, GORank *rank, unsigned midi_key_number)
    : GOPipe(handlerList, rank, midi_key_number) {}

  void Load(GOConfigReader &cfg, const wxString &group, const wxString &prefix)
    override {}
};

#endif
