/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEventHandlerList.h"

#include "control/GOControlChangedHandler.h"

void GOEventHandlerList::SendControlChanged(GOControl *pControl) {
  for (auto handler : mp_ControlChangedHandlers.AsVector())
    handler->ControlChanged(pControl);
}

void GOEventHandlerList::SetFrozen(bool isToFreeze) {
  mp_CacheObjects.SetFrozen(isToFreeze);
  mp_ReferencingObjects.SetFrozen(isToFreeze);
  mp_CombinationButtonSets.SetFrozen(isToFreeze);
  mp_MidiObjects.SetFrozen(isToFreeze);
  mp_MidiEventHandlers.SetFrozen(isToFreeze);
  mp_LifecycleListeners.SetFrozen(isToFreeze);
  mp_SaveableObjects.SetFrozen(isToFreeze);
}
