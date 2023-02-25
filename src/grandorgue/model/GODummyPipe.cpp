/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODummyPipe.h"

GODummyPipe::GODummyPipe(
  GOEventHandlerList *handlerList, GORank *rank, unsigned midi_key_number)
  : GOPipe(handlerList, rank, midi_key_number) {}

void GODummyPipe::Load(GOConfigReader &cfg, wxString group, wxString prefix) {}

void GODummyPipe::Change(unsigned velocity, unsigned old_velocity) {}
