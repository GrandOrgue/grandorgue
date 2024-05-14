/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOEventHandlerList.h"

#include <algorithm>

#include "control/GOControlChangedHandler.h"

void GOEventHandlerList::RegisterCacheObject(GOCacheObject *obj) {
  m_CacheObjects.push_back(obj);
}

void GOEventHandlerList::RegisterCombinationButtonSet(
  GOCombinationButtonSet *obj) {
  m_CombinationButtonSets.push_back(obj);
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

void GOEventHandlerList::RegisterSoundStateHandler(
  GOSoundStateHandler *handler) {
  m_SoundStateHandlers.push_back(handler);
}

void GOEventHandlerList::UnRegisterSoundStateHandler(
  GOSoundStateHandler *handler) {
  auto it = std::find(
    m_SoundStateHandlers.begin(), m_SoundStateHandlers.end(), handler);

  if (it != m_SoundStateHandlers.end()) {
    *it = nullptr;
    m_SoundStateHandlers.erase(it);
  }
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

void GOEventHandlerList::SendControlChanged(void *pControl) {
  for (auto handler : m_ControlChangedHandlers)
    handler->ControlChanged(pControl);
}

void GOEventHandlerList::Cleanup() {
  m_CacheObjects.clear();
  m_CombinationButtonSets.clear();
  m_ControlChangedHandlers.clear();
  m_MidiConfigurators.clear();
  m_MidiEventHandlers.clear();
  m_SoundStateHandlers.clear();
  m_SaveableObjects.clear();
}
