/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEventHandlerList.h"

#include <algorithm>

void GOEventHandlerList::RegisterCacheObject(GOCacheObject *obj) {
  m_CacheObjects.push_back(obj);
}

void GOEventHandlerList::RegisterControlChangedHandler(
  GOControlChangedHandler *handler) {
  m_ControlChangedHandlers.push_back(handler);
}

void GOEventHandlerList::RegisterEventHandler(GOEventHandler *handler) {
  m_MidiEventHandlers.push_back(handler);
}

void GOEventHandlerList::RegisterMidiConfigurator(GOMidiConfigurator *obj) {
  m_MidiConfigurators.push_back(obj);
}

void GOEventHandlerList::RegisterPlaybackStateHandler(
  GOPlaybackStateHandler *handler) {
  m_PlaybackStateHandlers.push_back(handler);
}

void GOEventHandlerList::RegisterSaveableObject(GOSaveableObject *obj) {
  if (
    std::find(m_SaveableObjects.begin(), m_SaveableObjects.end(), obj)
    == m_SaveableObjects.end())
    m_SaveableObjects.push_back(obj);
}

void GOEventHandlerList::UnregisterSaveableObject(GOSaveableObject *obj) {
  auto it = std::find(m_SaveableObjects.begin(), m_SaveableObjects.end(), obj);

  if (it != m_SaveableObjects.end()) {
    *it = nullptr;
    m_SaveableObjects.erase(it);
  }
}

void GOEventHandlerList::Cleanup() {
  m_CacheObjects.clear();
  m_ControlChangedHandlers.clear();
  m_MidiConfigurators.clear();
  m_MidiEventHandlers.clear();
  m_PlaybackStateHandlers.clear();
  m_SaveableObjects.clear();
}
