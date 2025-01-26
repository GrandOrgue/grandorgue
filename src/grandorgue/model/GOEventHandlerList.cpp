/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEventHandlerList.h"

#include "control/GOControlChangedHandler.h"

void GOEventHandlerList::SendControlChanged(GOControl *pControl) {
  for (auto handler : m_ControlChangedHandlers.AsVector())
    handler->ControlChanged(pControl);
}

void GOEventHandlerList::Cleanup() {
  m_CacheObjects.Clear();
  m_CombinationButtonSets.Clear();
  m_ControlChangedHandlers.Clear();
  m_MidiObjects.Clear();
  m_MidiEventHandlers.Clear();
  m_SoundStateHandlers.Clear();
  m_SaveableObjects.Clear();
}
